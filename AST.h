//
// Created by 储浩天 on 2020/7/16.
//

#ifndef BASIC_COMPILER_AST_H
#define BASIC_COMPILER_AST_H

#include "Instruction Generator.h"

class atom_AST{
    enum ATOM_TYPE{
        NUMBER, IDENTIFIER
    } atom_type;
public:
    int atom_value;
    string atom_name;
    atom_AST(int value_ = 0, string name_ = ""): atom_value(value_), atom_name(name_){
        if (atom_name == "!") atom_type = NUMBER;
        else atom_type = IDENTIFIER;
    }
    atom_AST(const atom_AST &other): atom_value(other.atom_value), atom_name(other.atom_name), atom_type(other.atom_type){}
    string generate_str(){
        if (atom_type == NUMBER) return to_string(atom_value);
        else return atom_name;
    }
};

class expr_AST{
    BINOP binop;
    unique_ptr<atom_AST> atom_expr;
    unique_ptr<expr_AST> LHS, RHS;
    vector<unique_ptr<expr_AST>> indexes;
    string expr_str = "";
public:
    expr_AST(){}
    string generate_str(){
        if (expr_str != "") return expr_str;
        string return_ = "";
        if (atom_expr){
            return_ += atom_expr->generate_str();
            for (int i = 0; i < indexes.size(); ++i){
                return_ += "[";
                return_ += indexes[i]->generate_str();
                return_ += "]";
            }
        }
        else {
            return_ += "(";
            return_ += LHS->generate_str();
            return_ += binop_str[binop];
            return_ += RHS->generate_str();
            return_ += ")";
        }
        return return_;
    }
    expr_AST(BINOP op, unique_ptr<expr_AST> LHS_, unique_ptr<expr_AST> RHS_)
                : binop(op), LHS(move(LHS_)), RHS(move(RHS_)){expr_str = this->generate_str();}
    expr_AST(unique_ptr<atom_AST> atom_expr_): atom_expr(move(atom_expr_)){expr_str = this->generate_str();}
    int value(){
        if (!atom_expr) return 0;
        else return atom_expr->atom_value;
    }
    string name(){
        if (!atom_expr) return "";
        else return atom_expr->atom_name;
    }
    void push(unique_ptr<expr_AST> index_){indexes.push_back(move(index_));}
    void generate_CFG(CFG_node &node){
        if (atom_expr) return;
        LHS->generate_CFG(node);
        RHS->generate_CFG(node);
        binop_ins(node, binop, LHS->generate_str(), RHS->generate_str());
    }
};

class let_AST{
    unique_ptr<expr_AST> lvalue;
    unique_ptr<expr_AST> rvalue;
public:
    let_AST(){}
    let_AST(unique_ptr<expr_AST> lvalue_, unique_ptr<expr_AST> rvalue_, unique_ptr<expr_AST> index_)
            : lvalue(move(lvalue_)), rvalue(move(rvalue_)){}
    void generate_CFG(CFG_node &node){
        lvalue->generate_CFG(node);
        rvalue->generate_CFG(node);
        assign_ins(node, lvalue->generate_str(), rvalue->generate_str());
    }
};

class input_AST{
    vector<unique_ptr<expr_AST>> identifiers;
public:
    input_AST(){}
    void push(unique_ptr<expr_AST> identifier){identifiers.push_back(move(identifier));}
    void generate_CFG(CFG_node &node){
        for (int i = 0; i < identifiers.size(); ++i){
            input_ins(node, identifiers[i]->generate_str());
        }
    }
};

class exit_AST{
    unique_ptr<expr_AST> exit_expr;
public:
    exit_AST(){}
    exit_AST(unique_ptr<expr_AST> exit_expr_)
            : exit_expr(move(exit_expr_)){}
    void generate_CFG(CFG_node &node){
        exit_expr->generate_CFG(node);
        return_ins(node, exit_expr->generate_str());
    }
};

class goto_AST{
public:
    int go_to;
    goto_AST(int go_to_ = 0): go_to(go_to_){}
    void generate_CFG(CFG_node &node){jump_ins(node, go_to);}
};

class if_AST;
class for_AST;
class stmt_AST{
public:
    unique_ptr<let_AST> let_stmt;
    unique_ptr<input_AST> input_stmt;
    unique_ptr<exit_AST> exit_stmt;
    unique_ptr<goto_AST> goto_stmt;
    unique_ptr<if_AST> if_stmt;
    unique_ptr<for_AST> for_stmt;
    bool isrem_ = 1;
    stmt_AST(){}
    stmt_AST(unique_ptr<stmt_AST> other)
: if_stmt(move(other->if_stmt)), for_stmt(move(other->for_stmt)), goto_stmt(move(other->goto_stmt))
, exit_stmt(move(other->exit_stmt)), input_stmt(move(other->input_stmt)), let_stmt(move(other->let_stmt)){isrem_ = other->isrem_;}
    stmt_AST(unique_ptr<let_AST> stmt_): let_stmt(move(stmt_)){isrem_ = 0;}
    stmt_AST(unique_ptr<input_AST> stmt_): input_stmt(move(stmt_)){isrem_ = 0;}
    stmt_AST(unique_ptr<exit_AST> stmt_): exit_stmt(move(stmt_)){isrem_ = 0;}
    stmt_AST(unique_ptr<goto_AST> stmt_): goto_stmt(move(stmt_)){isrem_ = 0;}
    stmt_AST(unique_ptr<if_AST> stmt_);
    stmt_AST(unique_ptr<for_AST> stmt_);
    bool isrem(){return isrem_;}
};

class if_AST{
    unique_ptr<expr_AST> if_expr;
public:
    unique_ptr<expr_AST> if_goto;
    if_AST(){}
    if_AST(unique_ptr<expr_AST> if_expr_, unique_ptr<expr_AST> if_goto_)
            :if_goto(move(if_goto_)), if_expr(move(if_expr_)){}
    void generate_CFG(CFG_node &node){
        if_expr->generate_CFG(node);
        if_goto->generate_CFG(node);
        branch_ins(node, if_expr->generate_str(), if_goto->value(), 1);
    }
};

void generate_CFG_(map<int, unique_ptr<stmt_AST>> stmts, int jump_);
class for_AST{
    map<int, unique_ptr<stmt_AST>> stmts;
    unique_ptr<stmt_AST> it_stmt;
    unique_ptr<expr_AST> continue_gate;
public:
    int for_line, after_end_for, end_for;
    for_AST(){}
    for_AST(unique_ptr<stmt_AST> it_stmt_, unique_ptr<expr_AST> continue_gate_, int for_line_)
            :it_stmt(move(it_stmt_)), continue_gate(move(continue_gate_)), for_line(for_line_){}
    void push(int line, unique_ptr<stmt_AST> stmt){stmts[line] = move(stmt);}
    void generate_CFG(){
        CFG_node node(after_end_for);
        branch_ins(node, "FOR_" + to_string(for_line), 0, 1);
        int size_1 = node.instructions.size();
        it_stmt->let_stmt->generate_CFG(node);
        int size_2 = node.instructions.size();
        int jump = (size_2 - size_1) * 4 + 4;
        uint32_t code = ((jump & 0x7fe) << 20) | (((jump >> 20) & 1) << 31) | (((jump >> 11) & 1) << 20) |
                        (((jump >> 12) & 0xff) << 12) | 0b1101111;
        node.instructions[size_1 - 1] = instruction(code, JAL, 0, 0, 0, jump);
        continue_gate->generate_CFG(node);
        ADDI_ins(node, 0, "FOR_" + to_string(for_line));
        branch_ins(node, continue_gate->generate_str(), after_end_for, 0);
        instruction tmp_jal = node.instructions[node.instructions.size() - 1];
        node.instructions.pop_back();
        int index_1 = node.instructions.size() - 1;
        ADDI_ins(node, 1, "FOR_" + to_string(for_line));
        node.instructions[index_1].code |= (1 << 9);
        node.instructions[index_1].imm = 12;
        node.instructions.push_back(tmp_jal);
        CFG[for_line] = node;
        generate_CFG_(move(stmts), -1);
        CFG_node node_(for_line);
        jump_ins(node_, for_line);
        CFG[end_for] = node_;
    }
};

stmt_AST::stmt_AST(unique_ptr<if_AST> stmt_)
            : if_stmt(move(stmt_)){isrem_ = 0;}
stmt_AST::stmt_AST(unique_ptr<for_AST> stmt_)
            : for_stmt(move(stmt_)){isrem_ = 0;}

class program_AST{
    map<int, unique_ptr<stmt_AST>> stmts;
public:
    void push(int line_, unique_ptr<stmt_AST> stmt){stmts[line_] = move(stmt);}
    void generate_CFG(){generate_CFG_(move(stmts), -2);}
};

bool for_store = 1;
void generate_CFG_(map<int, unique_ptr<stmt_AST>> stmts, int jump_){
    int addr, jump = 0;
    auto i = stmts.begin();
    while(i != stmts.end()){
        CFG_node node;
        if (for_store){
            for (auto j = for_lines.begin(); j != for_lines.end(); ++j){
                ADDI_ins(node, 1, j->first);
            }
            for_store = 0;
        }
        addr = i->first;
        if (i->second->let_stmt) {
            (i++)->second->let_stmt->generate_CFG(node);
            jump = -1;
        }
        else if (i->second->input_stmt) {
            (i++)->second->input_stmt->generate_CFG(node);
            jump = -1;
        }
        else if (i->second->if_stmt) {
            i->second->if_stmt->generate_CFG(node);
            jump = (i++)->second->if_stmt->if_goto->value();
        }
        else if (i->second->for_stmt) {
            (i++)->second->for_stmt->generate_CFG();
            continue;
        }
        else if (i->second->goto_stmt) {
            i->second->goto_stmt->generate_CFG(node);
            jump = (i++)->second->goto_stmt->go_to;
        }
        else if (i->second->exit_stmt) {
            (i++)->second->exit_stmt->generate_CFG(node);
            jump = -2;
        }
        if (i == stmts.end() && jump < 0) jump = jump_;
        node.jump = jump;
        CFG[addr] = node;
    }
}

#endif //BASIC_COMPILER_AST_H
