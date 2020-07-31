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
using namespace std;

enum OPCODE{
    LW, ADDI, TIMESI, DIVIDEI, SLTI, ORI, ANDI, XORI,
    SW, //S
    BEQ, BLT,
    LUI,
    JALR, //J(U)
    ADD_, SUB_, TIMES_, DIVIDE_, SLT, OR_, AND_, //R
    UNKNOWN
};

const char opcode_str[26][10]{
    "LW", "ADDI", "TIMESI", "DIVIDEI", "SLTI", "ORI", "ANDI", "XORI",
    "SW", //S
    "BEQ", "BLT",
    "LUI",
    "JAL", //J(U)
    "ADD_", "SUB_", "TIMES_", "DIVIDE_", "SLT", "OR_", "AND_", //R
    "UNKNOWN"
};

enum BINOP{
    ADD, SUB, TIMES,  DIVIDE, EQ, NOT_EQ, SMALLER, GREATER, SMALLER_EQ, GREATER_EQ, AND, OR
};

//map<string, int> val2reg;
map<int, string> reg2val;
map<string, int> val2mem;
map<int, string> mem2val;
class instruction{
public:
    uint32_t code;
    OPCODE opcode;
    int rd, rs1, rs2, imm;
    instruction(uint32_t code = 0, OPCODE opcode = UNKNOWN, int rd = 0, int rs1 = 0, int rs2 = 0, int imm = 0)
                : code(code), opcode(opcode), rd(rd), rs1(rs1), rs2(rs2), imm(imm){}
    void print_dump(){
        printf("%s\trd_%d\trs1_%d\trs2_%d\timm_%d\n", opcode_str[opcode], rd, rs1, rs2, imm);
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
    for(auto i = CFG.begin(); i != CFG.end(); ++i) {
        if (i->second.jump >= 0 || i->second.jump == -2) {
            printf("%d > %d\n", i->first, i->second.jump);
        } else printf("%d_", i->first);
    }
    printf("\n\n");
    for(auto i = CFG.begin(); i != CFG.end(); ++i){
//        if (i->second.jump) printf("%d > %d\n", i->first, i->second.jump);
//        else printf("%d_\n", i->first);
        for (int j = 0; j < i->second.instructions.size(); ++j){
            i->second.instructions[j].print_dump();
            //i->second.instructions[j].print_data();
            printf("\n");
        }
        if (i->second.jump >= 0 || i->second.jump == -2) printf("block_over\n\n");
    }
}

//-------------------------------------------------------------------------------------

int new_reg_ = 11, new_mem_space_ = 0x4000;
int new_reg(string val = ""){
    if (val == "!") return 10;
    if (val == ".") return 1;
    int reg = (new_reg_++) % 20 + 11;
    reg2val[reg] = val;
    return reg;
}
int new_mem_space(string val){
    if (val2mem.find(val) == val2mem.end()){
        int mem = new_mem_space_;
        new_mem_space_ += 4;
        val2mem[val] = mem, mem2val[mem] = val;
        return mem;
    } else return val2mem[val];
}
bool isdigit_(string x){
    if (!isdigit(x[0]) && x[0] != '-') return 0;
    for (int i = 1; i < strlen(x.c_str()); ++i){
        if (!isdigit(x[i])) return 0;
    }
    return 1;
}

void LOAD_ins(CFG_node &node, int rs, int rd){
    printf("LOAD, %s(%d)\n", reg2val[rd].c_str(), rd);
    uint32_t code = ((rs & 0b11111) << 15) | ((rd & 0b11111) << 7) | (1 << 13) | 0b0000011;
    node.instructions.push_back(instruction(code, LW, rd, rs, 0, 0));
}

void STORE_ins(CFG_node &node, int rs1, int rs2){
    printf("STORE, %s(%d)\n", reg2val[rs1].c_str(), rs1);
    uint32_t code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | (1 << 13) | 0b0100011;
    node.instructions.push_back(instruction(code, SW, 0, rs1, rs2, 0));
}

void ADDI_ins(CFG_node &node, int imm, string rd = "", int rs1 = 0){
    int rd_ = new_reg(rd);
    if (imm > 1024){
        int rd1 = new_reg("");
        uint32_t code = ((imm >> 12) << 12) | ((rd1 & 0b11111) << 7) | 0b0110111;
        node.instructions.push_back(instruction(code, LUI, rd1, rs1, 0, imm));
        int rd2 = new_reg("");
        code = (imm << 20) | ((rs1 & 0b11111) << 15) | ((rd2 & 0b11111) << 7) | 0b0010011;
        node.instructions.push_back(instruction(code, ADDI, rd2, rs1, 0, imm));
        code = ((rd2 & 0b11111) << 20) | ((rd1 & 0b11111) << 15) | ((rd_ & 0b11111) << 7) | 0b0110011;
        node.instructions.push_back(instruction(code, ADD_, rd_, rd1, rd2));
        return;
    }
    uint32_t code = (imm << 20) | ((rs1 & 0b11111) << 15) | ((rd_ & 0b11111) << 7) | 0b0010011;
    node.instructions.push_back(instruction(code, ADDI, rd_, rs1, 0, imm));
}

void BINOP_ins(CFG_node &node, BINOP binop, string LHS, string RHS){
    uint32_t code = 0;
    if (isdigit_(LHS)) ADDI_ins(node, strtod(LHS.c_str(), 0));
    else {
        ADDI_ins(node, new_mem_space(LHS));
        LOAD_ins(node, node.latest_rd(), new_reg());
    }
    int rs1 = node.latest_rd();
    if (isdigit_(RHS)) ADDI_ins(node, strtod(RHS.c_str(), 0));
    else {
        ADDI_ins(node, new_mem_space(RHS));
        LOAD_ins(node, node.latest_rd(), new_reg());
    }
    int rs2 = node.latest_rd(), rd = 0, rd1 = 0, rd2 = 0;
    switch (binop){
        case ADD:
            rd = new_reg("(" + LHS + "+" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, ADD_, rd, rs1, rs2));
            break;
        case SUB:
            rd = new_reg("(" + LHS + "-" + RHS + ")");
            code = (1 << 30) | ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, SUB_, rd, rs1, rs2));
            break;
        case TIMES:
            rd = new_reg("(" + LHS + "*" + RHS + ")");
            code = (1 << 29) | ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, TIMES_, rd, rs1, rs2));
            break;
        case DIVIDE:
            rd = new_reg("(" + LHS + "/" + RHS + ")");
            code = (1 << 28) | ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, DIVIDE_, rd, rs1, rs2));
            break;
        case EQ:
            rd = new_reg("(" + LHS + "==" + RHS + ")");
            rd1 = new_reg(), rd2 = new_reg();
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
            rd = new_reg("(" + LHS + "!=" + RHS + ")");
            rd1 = new_reg(), rd2 = new_reg();
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd1 & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd1, rs1, rs2));
            code = ((rs1 & 0b11111) << 20) | ((rs2 & 0b11111) << 15) | ((rd2 & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd2, rs2, rs1));
            code = ((rd2 & 0b11111) << 20) | ((rd1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (6 << 12);
            node.instructions.push_back(instruction(code, OR_, rd, rd1, rd2));
            break;
        case AND:
            rd = new_reg("(" + LHS + "&&" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (7 << 12);
            node.instructions.push_back(instruction(code, AND_, rd, rs1, rs2));
            break;
        case OR:
            rd = new_reg("(" + LHS + "||" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (6 << 12);
            node.instructions.push_back(instruction(code, OR_, rd, rs1, rs2));
            break;
        case GREATER:
            rd = new_reg("(" + LHS + ">" + RHS + ")");
            code = ((rs1 & 0b11111) << 20) | ((rs2 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs2, rs1));
            break;
        case GREATER_EQ:
            rd = new_reg("(" + LHS + ">=" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs1, rs2));
            code = (1 << 20) | ((rd & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0010011 | (4 << 12);
            node.instructions.push_back(instruction(code, XORI, rd, rd, 0, 1));
            break;
        case SMALLER:
            rd = new_reg("(" + LHS + "<" + RHS + ")");
            code = ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs1, rs2));
            break;
        case SMALLER_EQ:
            rd = new_reg("(" + LHS + "<=" + RHS + ")");
            code = ((rs1 & 0b11111) << 20) | ((rs2 & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs2, rs1));
            code = (1 << 20) | ((rd & 0b11111) << 15) | ((rd & 0b11111) << 7) | 0b0010011 | (4 << 12);
            node.instructions.push_back(instruction(code, XORI, rd, rd, 0, 1));
            break;
    }
    ADDI_ins(node, new_mem_space(reg2val[rd]));
    STORE_ins(node, node.latest_rd(), rd);
}

void MOVE_ins(CFG_node &node, string to, string from){
    ADDI_ins(node, new_mem_space(from));
    LOAD_ins(node, node.latest_rd(), new_reg());
    int addr = node.latest_rd();
    ADDI_ins(node, new_mem_space(to));
    STORE_ins(node, node.latest_rd(), addr);
}

void INPUT_ins(CFG_node &node, string to){
    int tmp = 0;
    scanf("%d", &tmp);
    ADDI_ins(node, tmp, to);
    int rs = node.latest_rd();
    ADDI_ins(node, new_mem_space(to));
    STORE_ins(node, node.latest_rd(), rs);
}

void EXIT_ins(CFG_node &node, string to){
    if (isdigit_(to)) ADDI_ins(node, strtod(to.c_str(), 0), "!");
    else {
        ADDI_ins(node, new_mem_space(to));
        LOAD_ins(node, node.latest_rd(), new_reg("!"));
    }
    node.instructions.push_back(instruction(0xff00513));
}

void GOTO_ins(CFG_node &node, int to){
    int rd = new_reg(".");
    uint32_t code = (to << 20)| ((rd & 0b11111) << 7) | 0b1100111;
    node.instructions.push_back(instruction(code, JALR, rd, 0, 0, to));
}

void BEQ_ins(CFG_node &node, string gate, int go_to, bool flag){
    ADDI_ins(node, new_mem_space(gate));
    LOAD_ins(node, node.latest_rd(), new_reg());
    int rs2 = node.latest_rd();
    uint32_t code;
    ADDI_ins(node, 0);
    int rs1 = node.latest_rd();
    if (flag){
        code = ((go_to >> 5) << 25) | ((go_to & 0b11110) << 7) | (go_to & (1 << 11)) >> 4 | (1 << 14) | 0b1100011;
        code |= ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15);
        node.instructions.push_back(instruction(code, BLT, 0, rs1, rs2, go_to));
    } else {
        code = ((go_to >> 5) << 25) | ((go_to & 0b11110) << 7) | (go_to & (1 << 11)) >> 4 | 0b1100011;
        code |= ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15);
        node.instructions.push_back(instruction(code, BEQ, 0, rs1, rs2, go_to));
    }
}

map<int, CFG_node> CFG_final;
int passed_ = 0;
void modify_CFG(){
    bool new_block = 1;
    int addr = 0, jump, cnt_print = 0;
    for(auto i = CFG.begin(); i != CFG.end(); ++i) {
        //printf("key: %d\n", i->first);
        addr = passed_;
        //printf("addr: %d\n", addr);
        passed_ += i->second.instructions.size() * 4;
        if (i->second.jump >= 0){
            jump = 0;
            for(auto k = CFG.begin(); k != CFG.end(); ++k) {
                if (k->first == i->second.jump) break;
                jump += k->second.instructions.size() * 4;
            }
            int index = i->second.instructions.size() - 1;
            int rs1 = i->second.instructions[index].rs1, rs2 = i->second.instructions[index].rs2, rd = i->second.instructions[index].rd;
            uint32_t code;
            switch(i->second.instructions[index].opcode){
                case BLT:
                    jump -= (passed_ - 4);
                    code = ((jump >> 5) << 25) | ((jump & 0b11110) << 7) | (jump & (1 << 11)) >> 4 | (1 << 14) | 0b1100011;
                    code |= ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15);
                    i->second.instructions[index] = instruction(code, BLT, rd, rs1, rs2, jump);
                    break;
                case BEQ:
                    jump -= (passed_ - 4);
                    code = ((jump >> 5) << 25) | ((jump  & 0b11110) << 7) | (jump  & (1 << 11)) >> 4 | 0b1100011;
                    code |= ((rs2 & 0b11111) << 20) | ((rs1 & 0b11111) << 15);
                    i->second.instructions[index] = instruction(code, BEQ, rd, rs1, rs2, jump);
                    break;
                case JALR:
                    code = (jump  << 20) | ((rd & 0b11111) << 7) | 0b1100111;
                    i->second.instructions[index] = instruction(code, JALR, rd, rs1, rs2, jump);
            }
        } else jump = i->second.jump;
        i->second.jump = jump;
        //printf("jump: %d\n", jump);
        CFG_final[addr] = i->second;
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
    printf("\ntotal: %08X\n", passed_);
}


#endif //BASIC_COMPLIER_INSTRUCTION_GENERATOR_H
