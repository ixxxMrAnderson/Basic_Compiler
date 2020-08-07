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
    INPUT, LW, ADDI, XORI, SW, BEQ, BLT, LUI, JAL, ADD_, SUB_, TIMES_, DIVIDE_, SLT, OR_, AND_, UNKNOWN
};

const char opcode_str[17][10]{
    "INPUT", "LW", "ADDI", "XORI", "SW", "BEQ", "BLT", "LUI", "JAL", "ADD_", "SUB_", "TIMES_", "DIVIDE_", "SLT", "OR_", "AND_", "UNKNOWN"
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

map<string, int> for_lines;
map<string, vector<int>> array_size;
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

void print_dump(){
    printf(".dump\n\n");
    for(auto i = CFG.begin(); i != CFG.end(); ++i){
        if (i->second.jump >= 0) printf("%d > %d\n", i->first, i->second.jump);
        else if (i->second.jump == -2) printf("%d > exit\n", i->first);
        else printf("%d_\n", i->first);
        for (int j = 0; j < i->second.instructions.size(); ++j){
            i->second.instructions[j].print_dump();
        }
        if (i->second.jump >= 0 || i->second.jump == -2) printf("\n\n");
    }
}

//-------------------------------------------------------------------------------------

int offset_ = 0;
int new_reg_ = 0, new_mem_space_ = 0x4000;
void ADDI_ins(CFG_node &node, int imm, string rd = "", int rs1 = 0, int rd__ = 0);
int new_reg(string val = ""){
    if (val == "return_") return 10; // return
    if (val == "addr_1") return 11; // tmp_addr_1
    if (val == "addr_2") return 12; // tmp_addr_2
    if (val == "gate_") return 13; // tmp_store for if/for gate
    if (val == "offset_") return 31; // offset
    if (for_lines.find(val) != for_lines.end()) return for_lines[val];
    int reg = (new_reg_++) % 17 + 14;
    reg2val[reg] = val;
    return reg;
}
void new_mem_space(CFG_node &node, string val, int new_space = 4, string rd = ""){
    if (val2mem.find(val) == val2mem.end()){
        int mem = new_mem_space_;
        new_mem_space_ += new_space;
        val2mem[val] = mem, mem2val[mem] = val;
        ADDI_ins(node, mem, rd);
    } else {
        ADDI_ins(node, val2mem[val], rd);
        if (offset_ > val2mem[val]) return;
    }
    if (offset_) {
        int rd_ = new_reg(rd);
        uint32_t code = (31 << 20) | (node.latest_rd() << 15) | ((rd_ & 0b11111) << 7) | 0b0110011;
        node.instructions.push_back(instruction(code, ADD_, rd_, node.latest_rd(), 31));
    }
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

void LW_ins(CFG_node &node, int rs, int rd, int imm = 0){
    uint32_t code = (imm << 20) | (rs << 15) | (rd << 7) | (1 << 13) | 0b0000011;
    node.instructions.push_back(instruction(code, LW, rd, rs, 0, imm));
}

void SW_ins(CFG_node &node, int rs1, int rs2, int imm = 0){
    uint32_t code = ((imm & 0b11111) << 7) | (((imm >> 5) & 0x7f) << 25) | (rs2 << 20) | (rs1 << 15) | (1 << 13) | 0b0100011;
    node.instructions.push_back(instruction(code, SW, 0, rs1, rs2, imm));
}

void BEQ_ins(CFG_node &node, int rs1, int rs2, int imm = 0){
    uint32_t code = (((imm >> 11) & 1) << 7) | (((imm >> 12) & 1) << 31) | (((imm >> 5) & 0x3f) << 25) | ((imm & 0b11110) << 7);
    code |= 0b1100011| (rs2 << 20) | (rs1 << 15);
    node.instructions.push_back(instruction(code, BEQ, 0, rs1, rs2, imm));
}

void BLT_ins(CFG_node &node, int rs1, int rs2, int imm = 0){
    uint32_t code = (((imm >> 11) & 1) << 7) | (((imm >> 12) & 1) << 31) | (((imm >> 5) & 0x3f) << 25) | ((imm & 0b11110) << 7);
    code |= (1 << 14) | 0b1100011| (rs2 << 20) | (rs1 << 15);
    node.instructions.push_back(instruction(code, BLT, 0, rs1, rs2, imm));
}

void ADDI_ins(CFG_node &node, int imm, string rd, int rs1, int rd__){
    int rd_ = 0;
    if (rd__) rd_ = rd__;
    else rd_ = new_reg(rd);
    if (imm > 1024){
        if (((imm >> 12) << 12) == imm){
            uint32_t code = ((imm >> 12) << 12) | (rd_ << 7) | 0b0110111;
            node.instructions.push_back(instruction(code, LUI, rd_, rs1, 0, imm));
            return;
        } else {
            int rd1 = new_reg();
            uint32_t code = ((imm >> 12) << 12) | (rd1 << 7) | 0b0110111;
            node.instructions.push_back(instruction(code, LUI, rd1, rs1, 0, imm));
            int rd2 = new_reg();
            code = (imm << 20) | (rs1 << 15) | (rd2 << 7) | 0b0010011;
            node.instructions.push_back(instruction(code, ADDI, rd2, rs1, 0, imm));
            code = (rd2 << 20) | (rd1 << 15) | (rd_ << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, ADD_, rd_, rd1, rd2));
            return;
        }
    }
    uint32_t code = (imm << 20) | (rs1 << 15) | (rd_ << 7) | 0b0010011;
    node.instructions.push_back(instruction(code, ADDI, rd_, rs1, 0, imm));
}

void JAL_ins(CFG_node &node, int jump){
    uint32_t code = ((jump & 0x7fe) << 20) | (((jump >> 20) & 1) << 31) | (((jump >> 11) & 1) << 20) | (((jump >> 12) & 0xff) << 12) | 0b1101111;
    node.instructions.push_back(instruction(code, JAL, 0, 0, 0, jump));
}

void binop_ins(CFG_node &node, BINOP binop, string LHS, string RHS, int RHS_rd, int index_1_, int default_rd = 0, int LHS_addr = 0){
    uint32_t code = 0;
    int index_2 = 0;
    if (isdigit_(LHS)) {
        ADDI_ins(node, strtod(LHS.c_str(), 0));
    } else {
        if (LHS_addr){
            new_mem_space(node, LHS + "_addr");
            LW_ins(node, node.latest_rd(), new_reg());
            LW_ins(node, node.latest_rd(), new_reg());
        } else {
            new_mem_space(node, LHS);
            LW_ins(node, node.latest_rd(), new_reg());
        }
    }
    int rs1 = node.latest_rd(), rs2 = RHS_rd, rd = 0, rd1 = 0, rd2 = 0, imm = 0;
    switch (binop){
        case ADD:
            if (!default_rd) rd = new_reg("(" + LHS + "+" + RHS + ")");
            else rd = default_rd;
            code = (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, ADD_, rd, rs1, rs2));
            break;
        case SUB:
            if (!default_rd) rd = new_reg("(" + LHS + "-" + RHS + ")");
            else rd = default_rd;
            code = (1 << 30) | (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, SUB_, rd, rs1, rs2));
            break;
        case TIMES:
            if (!default_rd) rd = new_reg("(" + LHS + "*" + RHS + ")");
            else rd = default_rd;
            code = (1 << 29) | (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, TIMES_, rd, rs1, rs2));
            break;
        case DIVIDE:
            if (!default_rd) rd = new_reg("(" + LHS + "/" + RHS + ")");
            else rd = default_rd;
            code = (1 << 28) | (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0b0110011;
            node.instructions.push_back(instruction(code, DIVIDE_, rd, rs1, rs2));
            break;
        case EQ:
            if (!default_rd) rd = new_reg("(" + LHS + "==" + RHS + ")");
            else rd = default_rd;
            rd1 = new_reg(), rd2 = new_reg();
            code = (rs2 << 20) | (rs1 << 15) | (rd1 << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd1, rs1, rs2));
            code = (rs1 << 20) | (rs2 << 15) | (rd2 << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd2, rs2, rs1));
            code = (rd2 << 20) | (rd1 << 15) | (rd << 7) | 0b0110011 | (6 << 12);
            node.instructions.push_back(instruction(code, OR_, rd, rd1, rd2));
            code = (1 << 20) | (rd << 15) | (rd << 7) | 0b0010011 | (4 << 12);
            node.instructions.push_back(instruction(code, XORI, rd, rd, 0, 1));
            break;
        case NOT_EQ:
            if (!default_rd) rd = new_reg("(" + LHS + "!=" + RHS + ")");
            else rd = default_rd;
            rd1 = new_reg(), rd2 = new_reg();
            code = (rs2 << 20) | (rs1 << 15) | (rd1 << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd1, rs1, rs2));
            code = (rs1 << 20) | (rs2 << 15) | (rd2 << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd2, rs2, rs1));
            code = (rd2 << 20) | (rd1 << 15) | (rd << 7) | 0b0110011 | (6 << 12);
            node.instructions.push_back(instruction(code, OR_, rd, rd1, rd2));
            break;
        case AND:
            if (!default_rd) rd = new_reg("(" + LHS + "&&" + RHS + ")");
            else rd = default_rd;
            code = (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0b0110011 | (7 << 12);
            node.instructions.push_back(instruction(code, AND_, rd, rs1, rs2));
            break;
        case OR:
            if (!default_rd) rd = new_reg("(" + LHS + "||" + RHS + ")");
            else rd = default_rd;
            code = (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0b0110011 | (6 << 12);
            node.instructions.push_back(instruction(code, OR_, rd, rs1, rs2));
            break;
        case GREATER:
            if (!default_rd) rd = new_reg("(" + LHS + ">" + RHS + ")");
            else rd = default_rd;
            code = (rs1 << 20) | (rs2 << 15) | (rd << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs2, rs1));
            break;
        case GREATER_EQ:
            if (!default_rd) rd = new_reg("(" + LHS + ">=" + RHS + ")");
            else rd = default_rd;
            code = (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs1, rs2));
            code = (1 << 20) | (rd << 15) | (rd << 7) | 0b0010011 | (4 << 12);
            node.instructions.push_back(instruction(code, XORI, rd, rd, 0, 1));
            break;
        case SMALLER:
            if (!default_rd) rd = new_reg("(" + LHS + "<" + RHS + ")");
            else rd = default_rd;
            code = (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs1, rs2));
            break;
        case SMALLER_EQ:
            if (!default_rd) rd = new_reg("(" + LHS + "<=" + RHS + ")");
            else rd = default_rd;
            code = (rs1 << 20) | (rs2 << 15) | (rd << 7) | 0b0110011 | (2 << 12);
            node.instructions.push_back(instruction(code, SLT, rd, rs2, rs1));
            code = (1 << 20) | (rd << 15) | (rd << 7) | 0b0010011 | (4 << 12);
            node.instructions.push_back(instruction(code, XORI, rd, rd, 0, 1));
            break;
    }
//    if (binop == AND || binop == OR){
//        JAL_ins(node, 8);
//        index_2 = node.instructions.size() - 1;
//        if (binop == AND) ADDI_ins(node, 0, "", 0, rd);
//        else ADDI_ins(node, 1, "", 0, rd);
//        int jump = (index_2 - index_1_) * 4 + 4;
//        uint32_t code = ((jump & 0x7fe) << 20) | (((jump >> 20) & 1) << 31) | (((jump >> 11) & 1) << 20) |
//                        (((jump >> 12) & 0xff) << 12) | 0b1101111;
//        node.instructions[index_1_] = instruction(code, JAL, 0, 0, 0, jump);
//    }
    if (default_rd) return;
    new_mem_space(node, reg2val[rd]);
    SW_ins(node, node.latest_rd(), rd);
}

int passed_ = 0;
void generate_code(){
    printf(".data\n\n");
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
            JAL_ins(i->second, jump);
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
