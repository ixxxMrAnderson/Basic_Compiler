//
// Created by 储浩天 on 2020/7/16.
//

#ifndef BASIC_COMPILER_AST_H
#define BASIC_COMPILER_AST_H

#include "Instruction Generator.h"

enum TOKEN_TYPE{
    EOF_ = -1, number_ = -2, identifier_ = -3, binop_ = -4,
    LET_ = -5, INPUT_ = -6, EXIT_ = -7, GOTO_ = -8, IF_ = -9, THEN_ = -10, FOR_ = -11, END_ = -12, REM_ = -13,
    line_ = -14
};

const char binop_str[12][3]{
    "+", "-", "*", "/", "==", "!=", "<", ">", "<=", ">=", "&&","||"
};

class atom_AST{
public:
    enum ATOM_TYPE{
        NUMBER, IDENTIFIER
    } atom_type;
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
        BINOP_ins(node, binop, LHS->generate_str(), RHS->generate_str());
    }
};

class let_AST{
    unique_ptr<expr_AST> lvalue;
    unique_ptr<expr_AST> rvalue;
public:
    let_AST(){}
    let_AST(unique_ptr<expr_AST> lvalue_, unique_ptr<expr_AST> rvalue_, unique_ptr<expr_AST> index_)
            : lvalue(move(lvalue_)), rvalue(move(rvalue_)){}
    void generate(){
        printf("LET ");
        printf("%s", lvalue->generate_str().c_str());
        printf(" = ");
        printf("%s", rvalue->generate_str().c_str());
        printf("\n");
    }
    void generate_CFG(CFG_node &node){
        lvalue->generate_CFG(node);
        rvalue->generate_CFG(node);
        MOVE_ins(node, lvalue->generate_str(), rvalue->generate_str());
    }
};

class input_AST{
    vector<unique_ptr<expr_AST>> identifiers;
public:
    input_AST(){}
    void push(unique_ptr<expr_AST> identifier){identifiers.push_back(move(identifier));}
    void generate(){
        printf("INPUT ");
        for (int i = 0; i < identifiers.size(); ++i){
            auto iden_ = identifiers[i]->name().c_str();
            if (iden_ == ""){
                printf("Failed. INPUT statement must follow a list of identifiers.\n");
                exit(0);
            }
            printf("%s", iden_);
            if (i == identifiers.size() - 1) printf("\n");
            else printf(", ");
        }
    }
    void generate_CFG(CFG_node &node){
        for (int i = 0; i < identifiers.size(); ++i){
            INPUT_ins(node, identifiers[i]->generate_str());
        }
    }
};

class exit_AST{
    unique_ptr<expr_AST> exit_expr;
public:
    exit_AST(){}
    exit_AST(unique_ptr<expr_AST> exit_expr_)
            : exit_expr(move(exit_expr_)){}
    void generate(){
        printf("EXIT ");
        printf("%s", exit_expr->generate_str().c_str());
        printf("\n");
    }
    void generate_CFG(CFG_node &node){
        exit_expr->generate_CFG(node);
        EXIT_ins(node, exit_expr->generate_str());
    }
};

class goto_AST{
public:
    int go_to;
    goto_AST(int go_to_ = 0): go_to(go_to_){}
    void generate(){printf("GOTO %d\n", go_to);}
    void generate_CFG(CFG_node &node){GOTO_ins(node, go_to);}
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
    void generate();
};

class if_AST{
public:
    unique_ptr<expr_AST> if_expr;
    unique_ptr<expr_AST> if_goto;
    if_AST(){}
    if_AST(unique_ptr<expr_AST> if_expr_, unique_ptr<expr_AST> if_goto_)
            :if_goto(move(if_goto_)), if_expr(move(if_expr_)){}
    void generate(){
        printf("IF ");
        printf("%s", if_expr->generate_str().c_str());
        printf(" THEN ");
        printf("%s", if_goto->generate_str().c_str());
        printf("\n");
    }
    void generate_CFG(CFG_node &node){
        if_expr->generate_CFG(node);
        if_goto->generate_CFG(node);
        BEQ_ins(node, if_expr->generate_str(), if_goto->value(), 1);
    }
};

void generate_CFG_(map<int, unique_ptr<stmt_AST>> stmts, int jump_);
class for_AST{
public:
    map<int, unique_ptr<stmt_AST>> stmts;
    unique_ptr<stmt_AST> it_stmt;
    unique_ptr<expr_AST> continue_gate;
    int for_line, after_end_for, end_for;
    for_AST(){}
    for_AST(unique_ptr<stmt_AST> it_stmt_, unique_ptr<expr_AST> continue_gate_, int for_line_)
            :it_stmt(move(it_stmt_)), continue_gate(move(continue_gate_)), for_line(for_line_){}
    void push(int line, unique_ptr<stmt_AST> stmt){stmts[line] = move(stmt);}
    void generate(){
        printf("FOR ");
        it_stmt->generate();
        printf("%s\n", continue_gate->generate_str().c_str());
        for (auto i = stmts.begin(); i != stmts.end(); ++i){
            printf("%d ", i->first);
            i->second->generate();
        }
        printf("%d END FOR\n", end_for);
    }
    void generate_CFG(){
        CFG_node node(after_end_for);
        it_stmt->let_stmt->generate_CFG(node);
        continue_gate->generate_CFG(node);
        BEQ_ins(node, continue_gate->generate_str(), after_end_for, 0);
        printf("(jump_%d)\n", after_end_for);
        CFG[for_line] = node;
        generate_CFG_(move(stmts), end_for);
        CFG_node node_(for_line);
        GOTO_ins(node_, for_line);
        CFG[end_for] = node_;
        printf("(addr_%d)\nENDFOR\n", end_for);
        printf("(jump_%d)\n", for_line);
    }
};

stmt_AST::stmt_AST(unique_ptr<if_AST> stmt_)
            : if_stmt(move(stmt_)){isrem_ = 0;}
stmt_AST::stmt_AST(unique_ptr<for_AST> stmt_)
            : for_stmt(move(stmt_)){isrem_ = 0;}

void stmt_AST::generate(){
    if (let_stmt) return let_stmt->generate();
    if (input_stmt) return input_stmt->generate();
    if (exit_stmt) return exit_stmt->generate();
    if (goto_stmt) return goto_stmt->generate();
    if (if_stmt) return if_stmt->generate();
    if (for_stmt) return for_stmt->generate();
}

class program_AST{
    map<int, unique_ptr<stmt_AST>> stmts;
public:
    void push(int line_, unique_ptr<stmt_AST> stmt){stmts[line_] = move(stmt);}
    void generate(){
        for (auto i = stmts.begin(); i != stmts.end(); ++i){
            printf("%d ", i->first);
            i->second->generate();
        }
    }
    void generate_CFG(){generate_CFG_(move(stmts), -2);}
};

void generate_CFG_(map<int, unique_ptr<stmt_AST>> stmts, int jump_){
    int addr, jump = 0;
    auto i = stmts.begin();
    while(i != stmts.end()){
        CFG_node node;
        addr = i->first;
        printf("(addr_%d)\n", addr);
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
        printf("(jump_%d)\n", jump);
        node.jump = jump;
        CFG[addr] = node;
    }
}

#endif //BASIC_COMPILER_AST_H
