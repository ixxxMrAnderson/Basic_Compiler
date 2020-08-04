//
// Created by 储浩天 on 2020/7/27.
//

#ifndef BASIC_COMPLIER_INSTRUCTION_GENERATOR_H
#define BASIC_COMPLIER_INSTRUCTION_GENERATOR_H

#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <cstring>
#include <iostream>
using namespace std;

enum OPCODE{
    LW, ADDI, XORI, SW, BEQ, BLT, LUI, JAL, ADD_, SUB_, TIMES_, DIVIDE_, SLT, OR_, AND_, UNKNOWN
};

const char opcode_str[16][10]{
        "LW", "ADDI", "XORI", "SW", "BEQ", "BLT", "LUI", "JAL", "ADD_", "SUB_", "TIMES_", "DIVIDE_", "SLT", "OR_", "AND_", "UNKNOWN"
};

enum BINOP{
    ADD, SUB, TIMES,  DIVIDE, EQ, NOT_EQ, SMALLER, GREATER, SMALLER_EQ, GREATER_EQ, AND, OR
};

enum TOKEN_TYPE{
    EOF_ = -1, number_ = -2, identifier_ = -3, binop_ = -4,
    LET_ = -5, INPUT_ = -6, EXIT_ = -7, GOTO_ = -8, IF_ = -9, THEN_ = -10, FOR_ = -11, END_ = -12, REM_ = -13,
    line_ = -14
};

const char binop_str[12][3]{
        "+", "-", "*", "/", "==", "!=", "<", ">", "<=", ">=", "&&","||"
};

vector<int> for_lines;
map<int, string> reg2val;
map<string, int> val2reg;
map<int, string> mem2val;
map<string, int> val2mem;
class instruction{
public:
    uint32_t code;
    OPCODE opcode;
    int rd, rs1, rs2, imm;
    instruction(uint32_t code = 0, OPCODE opcode = UNKNOWN, int rd = 0, int rs1 = 0, int rs2 = 0, int imm = 0)
            : code(code), opcode(opcode), rd(rd), rs1(rs1), rs2(rs2), imm(imm){}
    void print_dump(){
        printf("%s\trd_%d\trs1_%d\trs2_%d\timm_%d\n", opcode_str[opcode], rd, rs1, rs2, imm);
        //printf("%s\trd_%d(%s)\trs1_%d(%s)\trs2_%d(%s)\timm_%d\n", opcode_str[opcode], rd, reg2val[rd].c_str(), rs1, reg2val[rs1].c_str(), rs2, reg2val[rs2].c_str(), imm);
    }
    void print_data(){
        uint32_t bin = 0xff;
        printf("%02X %02X %02X %02X ", code & bin, (code & (bin << 8)) >> 8, (code & (bin << 16)) >> 16, (code & (bin << 24)) >> 24);
    }
};

class CFG_node{
public:
    vector<instruction> instructions;
    int jump = 0;
    CFG_node(vector<instruction> instructions, int jump = 0)
            :instructions(instructions), jump(jump){}
    CFG_node(int jump = 0): jump(jump){}
    int latest_rd(){return instructions[instructions.size() - 1].rd;}
};

map<int, CFG_node> CFG;
void print_CFG(){
    printf("\nCFG_\n");
    for(auto i = CFG.begin(); i != CFG.end(); ++i) {
        if (i->second.jump >= 0) {
            printf("%d > %d\n", i->first, i->second.jump);
        } else if (i->second.jump == -2){
            printf("%d > exit\n", i->first);
        } else printf("%d_", i->first);
    }
    printf("\n\n");
    for(auto i = CFG.begin(); i != CFG.end(); ++i){
        if (i->second.jump) printf("%d > %d\n", i->first, i->second.jump);
        else printf("%d_\n", i->first);
        for (int j = 0; j < i->second.instructions.size(); ++j){
            i->second.instructions[j].print_dump();
        }
        if (i->second.jump >= 0 || i->second.jump == -2) printf("\n\n");
    }
}

//-------------------------------------------------------------------------------------

int new_reg_ = 0, new_mem_space_ = 0x4000;
int avail[32]{1};
int new_mem_space(string val){
    if (val2mem.find(val) != val2mem.end()) return val2mem[val];
    int mem = new_mem_space_;
    new_mem_space_ += 4;
    val2mem[val] = mem, mem2val[mem] = val;
    return mem;
}
void STORE_ins(CFG_node &node, int rs1, int rs2, int imm = 0);
void ADDI_ins(CFG_node &node, int imm, string rd = "", int rs1 = 0);
void free_reg(int reg){
    printf("free: %d(%s)\n", reg, reg2val[reg].c_str());
    avail[reg] = 1;
    val2reg.erase(reg2val[reg]);
    reg2val.erase(reg);
}
int new_reg(CFG_node &node, string val = ""){
    printf("new_reg: %s\n", val.c_str());
    if (val == "!") return 10;
    if (val2reg.find(val) != val2reg.end()) return val2reg[val];
    new_reg_++;
    if (new_reg_ % 32 == 10 || new_reg_ % 32 == 0) new_reg_ ++;
    int reg = new_reg_ % 32;
    if (!avail[reg]){
        printf("reg%d(%s) not available\n", reg, reg2val[reg].c_str());
        int addr = new_mem_space(reg2val[reg]);
        printf("STORE %s on %d\n", mem2val[addr].c_str(), addr);
        ADDI_ins(node, (addr >> 12) << 12, "!");
        STORE_ins(node, node.latest_rd(), reg, addr - ((addr >> 12) << 12));
        free_reg(10);
        free_reg(reg);
    }
    if (val != "") reg2val[reg] = val, val2reg[val] = reg, avail[reg] = 0;
    return reg;
}
bool isdigit_(string x){
    if (!isdigit(x[0]) && x[0] != '-') return 0;
    for (int i = 1; i < strlen(x.c_str()); ++i){
        if (!isdigit(x[i])) return 0;
    }
    return 1;
}
void log_error(const char *text_){
    printf("error: %s\n", text_);
    exit(0);
}

void LOAD_ins(CFG_node &node, int rs, int rd, int imm = 0){
    uint32_t code = (imm << 20) | ((rs & 0b11111) << 15) | ((rd & 0b11111) << 7) | (1 << 13) | 0b0000011;
    node.instructions.push_back(instruction(code, LW, rd, rs, 0, imm));
}

void STORE_ins(CFG_node &node, int rs1, int rs2, int imm){
    uint32_t code = ((imm & 0b11111) << 7) | (((imm >> 5) & 0x7f) << 25) | ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | (1 << 13) | 0b0100011;
    node.instructions.push_back(instruction(code, SW, 0, rs1, rs2, imm));
}

void ADDI_ins(CFG_node &node, int imm, string rd, int rs1){
    printf("ADDI_rd_%s\n", rd.c_str());
    int rd_ = 0;
    if (val2reg.find(rd) == val2reg.end()) rd_ = new_reg(node, rd);
    else rd_ = val2reg[rd];
    if (imm > 1024){
        if (((imm >> 12) << 12) == imm){
            uint32_t code = ((imm >> 12) << 12) | ((rd_ & 0b11111) << 7) | 0b0110111;
            node.instructions.push_back(instruction(code, LUI, rd_, rs1, 0, imm));
            return;
        } else {
            int rd1 = new_reg(node);
            uint32_t code = ((imm >> 12) << 12) | ((rd1 & 0b11111) << 7) | 0b0110111;
            node.instructions.push_back(instruction(code, LUI, rd1, rs1, 0, imm));
            int rd2 = new_reg(node);
            code = (imm << 20) | ((rs1 & 0b11111) << 15) | ((rd2 & 0b11111) << 7) | 0b0010011;
            node.instructions.push_back(instruction(code, ADDI, rd2, rs1, 0, imm));
            code = ((rd2 & 0b11111) << 20) | ((rd1 & 0b11111) << 15) | ((rd_ & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, ADD_, rd_, rd1, rd2));
            return;
        }
    }
    uint32_t code = (imm << 20) | ((rs1 & 0b11111) << 15) | ((rd_ & 0b11111) << 7) | 0b0010011;
    node.instructions.push_back(instruction(code, ADDI, rd_, rs1, 0, imm));
}

void binop_ins(CFG_node &node, BINOP binop, string LHS, string RHS){
    printf("in %s %s %s\n", LHS.c_str(), binop_str[binop], RHS.c_str());
    uint32_t code = 0;
    int addr = 0, rs1 = 0, rs2 = 0;
    if (isdigit_(LHS)) {
        ADDI_ins(node, strtod(LHS.c_str(), 0));
        rs1 = node.latest_rd();
    } else {
        if (val2reg.find(LHS) == val2reg.end()) {
            addr = val2mem[LHS];
            ADDI_ins(node, (addr >> 12) << 12);
            LOAD_ins(node, node.latest_rd(), new_reg(node), addr - ((addr >> 12) << 12));
            rs1 = node.latest_rd();
        } else rs1 = val2reg[LHS];
    }
//    if (binop == AND_){
//
//    }
//    if (binop == OR_){
//
//    }
    if (isdigit_(RHS)) {
        ADDI_ins(node, strtod(RHS.c_str(), 0));
        rs2 = node.latest_rd();
    }
    else {
        if (val2reg.find(RHS) == val2reg.end()) {
            addr = val2mem[RHS];
            ADDI_ins(node, (addr >> 12) << 12);
            LOAD_ins(node, node.latest_rd(), new_reg(node), addr - ((addr >> 12) << 12));
            rs2 = node.latest_rd();
        } else rs2 = val2reg[RHS];
    }
    int rd = 0, rd1 = 0, rd2 = 0;
    switch (binop){
        case ADD:
            rd = new_reg(node, "(" + LHS + "+" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, ADD_, rd, rs1, rs2));
            break;
        case SUB:
            rd = new_reg(node, "(" + LHS + "-" + RHS + ")");
            code = (1 << 30) | ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, SUB_, rd, rs1, rs2));
            break;
        case TIMES:
            rd = new_reg(node, "(" + LHS + "*" + RHS + ")");
            code = (1 << 29) | ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, TIMES_, rd, rs1, rs2));
            break;
        case DIVIDE:
            rd = new_reg(node, "(" + LHS + "/" + RHS + ")");
            code = (1 << 28) | ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, DIVIDE_, rd, rs1, rs2));
            break;
        case EQ:
            rd = new_reg(node, "(" + LHS + "==" + RHS + ")");
            rd1 = new_reg(node), rd2 = new_reg(node);
            avail[rd1] = avail[rd2] = 1;
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd1 & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd1, rs1, rs2));
            code = ((rs1 & 0b11111) << 20) | ((rs2 & 0b11111) << 15) | ((rd2 & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd2, rs2, rs1));
            code = ((rd2 & 0b11111) << 20) | ((rd1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (6 << 12);
            node.instructions.push_back(instruction(code, OR_, rd, rd1, rd2));
            code = (1 << 20) | ((rd & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0010011 | (4 << 12);
            node.instructions.push_back(instruction(code, XORI, rd, rd, 0, 1));
            break;
        case NOT_EQ:
            rd = new_reg(node, "(" + LHS + "!=" + RHS + ")");
            rd1 = new_reg(node), rd2 = new_reg(node);
            avail[rd1] = avail[rd2] = 1;
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd1 & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd1, rs1, rs2));
            code = ((rs1 & 0b11111) << 20) | ((rs2 & 0b11111) << 15) | ((rd2 & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd2, rs2, rs1));
            code = ((rd2 & 0b11111) << 20) | ((rd1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (6 << 12);
            node.instructions.push_back(instruction(code, OR_, rd, rd1, rd2));
            break;
        case AND:
            rd = new_reg(node, "(" + LHS + "&&" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (7 << 12);
            node.instructions.push_back(instruction(code, AND_, rd, rs1, rs2));
            break;
        case OR:
            rd = new_reg(node, "(" + LHS + "||" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (6 << 12);
            node.instructions.push_back(instruction(code, OR_, rd, rs1, rs2));
            break;
        case GREATER:
            rd = new_reg(node, "(" + LHS + ">" + RHS + ")");
            code = ((rs1 & 0b11111) << 20) | ((rs2 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs2, rs1));
            break;
        case GREATER_EQ:
            rd = new_reg(node, "(" + LHS + ">=" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs1, rs2));
            code = (1 << 20) | ((rd & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0010011 | (4 << 12);
            node.instructions.push_back(instruction(code, XORI, rd, rd, 0, 1));
            break;
        case SMALLER:
            rd = new_reg(node, "(" + LHS + "<" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs1, rs2));
            break;
        case SMALLER_EQ:
            rd = new_reg(node, "(" + LHS + "<=" + RHS + ")");
            code = ((rs1 & 0b11111) << 20) | ((rs2 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs2, rs1));
            code = (1 << 20) | ((rd & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0010011 | (4 << 12);
            node.instructions.push_back(instruction(code, XORI, rd, rd, 0, 1));
            break;
    }
    printf("end %s %s %s\n", LHS.c_str(), binop_str[binop], RHS.c_str());
}

void assign_ins(CFG_node &node, string to, string from){
    int rs = 0, addr = 0;
    if (isdigit_(from)) {
        ADDI_ins(node, strtod(from.c_str(), 0), to);
    } else {
        if (val2reg.find(from) == val2reg.end()) {
            addr = val2mem[from];
            ADDI_ins(node, (addr >> 12) << 12);
            LOAD_ins(node, node.latest_rd(), new_reg(node), addr - ((addr >> 12) << 12));
            rs = node.latest_rd();
        } else rs = val2reg[from];
        ADDI_ins(node, 0, to, rs);
    }
}

void input_ins(CFG_node &node, string to){
    int tmp = 0;
    printf("input %s:\n", to.c_str());
    scanf("%d", &tmp);
    ADDI_ins(node, tmp, to);
}

void return_ins(CFG_node &node, string to){
    if (isdigit_(to)) ADDI_ins(node, strtod(to.c_str(), 0), "!");
    else {
        if (val2reg.find(to) == val2reg.end()) {
            int addr = val2mem[to];
            ADDI_ins(node, (addr >> 12) << 12);
            LOAD_ins(node, node.latest_rd(), new_reg(node, "!"), addr - ((addr >> 12) << 12));
        } else ADDI_ins(node, 0, "!", val2reg[to]);
    }
    node.instructions.push_back(instruction(0xff00513));
}

void jump_ins(CFG_node &node, int jump){
    uint32_t code = ((jump & 0x7fe) << 20) | (((jump >> 20) & 1) << 31) | (((jump >> 11) & 1) << 20) | (((jump >> 12) & 0xff) << 12) | 0b1101111;
    node.instructions.push_back(instruction(code, JAL, 0, 0, 0, jump));
}

void branch_ins(CFG_node &node, string gate, int jump, bool flag){
    int rs2 = 0;
    if (val2reg.find(gate) == val2reg.end()) {
        int addr = val2mem[gate];
        ADDI_ins(node, (addr >> 12) << 12);
        LOAD_ins(node, node.latest_rd(), new_reg(node, "!"), addr - ((addr >> 12) << 12));
        rs2 = node.latest_rd();
    } else rs2 = val2reg[gate];
    uint32_t code;
    ADDI_ins(node, 0);
    int rs1 = node.latest_rd();
    if (flag){
        code = (1 << 10) | 0b1100011| ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15);
        node.instructions.push_back(instruction(code, BEQ, 0, rs1, rs2, 8));
    } else {
        code = (1 << 10) | (1 << 14) | 0b1100011| ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15);
        node.instructions.push_back(instruction(code, BLT, 0, rs1, rs2, 8));
    }
    jump_ins(node, jump);
}

int passed_ = 0;
void generate_code(){
    printf("\n\n");
    bool new_block = 1;
    int addr = 0, jump, cnt_print = 0;
    for(auto i = CFG.begin(); i != CFG.end(); ++i) {
        addr = passed_;
        passed_ += i->second.instructions.size() * 4;
        if (i->second.jump >= 0) {
            jump = 0;
            for (auto k = CFG.begin(); k != CFG.end(); ++k) {
                if (k->first == i->second.jump) break;
                jump += k->second.instructions.size() * 4;
            }
            jump -= (passed_ - 4);
            i->second.instructions.pop_back();
            jump_ins(i->second, jump);
        }
        if (new_block) {
            printf("@%08X\n", addr);
            new_block = 0;
            cnt_print = 0;
        }
        for (int j = 0; j < i->second.instructions.size(); ++j){
            i->second.instructions[j].print_data();
            if ((++cnt_print) % 4 == 0) printf("\n");
        }
        if (i->second.jump >= 0){
            new_block = 1;
            printf("\n\n");
        }
    }
    printf("\n\nmemory_space_used_%08X(byte)\n", passed_);
}


#endif //BASIC_COMPLIER_INSTRUCTION_GENERATOR_H