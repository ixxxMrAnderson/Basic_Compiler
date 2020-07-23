//
// Created by 储浩天 on 2020/7/16.
//

#ifndef BASIC_COMPILER_AST_H
#define BASIC_COMPILER_AST_H

#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <cstring>
using namespace std;

enum TOKEN_TYPE{
    EOF_ = -1, number_ = -2, identifier_ = -3, binop_ = -4,
    LET_ = -5, INPUT_ = -6, EXIT_ = -7, GOTO_ = -8, IF_ = -9, THEN_ = -10, FOR_ = -11, END_ = -12, REM_ = -13,
    line_ = -14
};
enum BINOP{
    ADD, SUB, TIMES,  DIVIDE, EQ, NOT_EQ, SMALLER, GREATER, SMALLER_EQ, GREATER_EQ, AND, OR
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
    atom_AST(const atom_AST &other): atom_value(other.atom_value), atom_name(other.atom_name){}
    bool isnumber(){return atom_type == NUMBER;}
    bool isidentifier(){return atom_type == IDENTIFIER;}
    atom_AST &operator=(const atom_AST &other){
        printf("atom operator=\n");
        if (&other == this) return *this;
        atom_value = other.atom_value;
        atom_name = other.atom_name;
        printf("atom operator= return\n");
        return *this;
    }
};

class expr_AST{
public:
    BINOP binop;
    unique_ptr<atom_AST> atom_expr;
    unique_ptr<expr_AST> LHS, RHS;
    expr_AST(){}
    expr_AST(BINOP op, unique_ptr<expr_AST> LHS_, unique_ptr<expr_AST> RHS_): binop(op){
        if (LHS_ != NULL) *LHS = *LHS_;
        if (RHS_ != NULL) *RHS = *RHS_;
        atom_expr = NULL;
    }
    expr_AST(unique_ptr<atom_AST> atom_expr_){
        printf("expr_atom constructor\n");
        if (atom_expr == NULL) printf("wrong NULL\n");
        if (atom_expr_ != NULL) *atom_expr = *atom_expr_;
        printf("expr_atom constructor\n");
        LHS = NULL, RHS = NULL;
    }
    expr_AST(const expr_AST &other): binop(other.binop){
        printf("constructor\n");
        other.print_();
        if (other.atom_expr != NULL){
            *atom_expr = *other.atom_expr;
            LHS = NULL, RHS = NULL;
        } else {
            if (other.LHS != NULL) *LHS = *other.LHS;
            if (other.RHS != NULL) *RHS = *other.RHS;
            atom_expr = NULL;
        }
    }
    bool isatom() const{return atom_expr != NULL;}
    int value(){return atom_expr->atom_value;}
    string name(){return atom_expr->atom_name;}
    expr_AST &operator=(const expr_AST &other){
        printf("operator=\n");
        if (&other == this) return *this;
        binop = other.binop;
        if (other.atom_expr != NULL){
            *atom_expr = *other.atom_expr;
            LHS = NULL, RHS = NULL;
        } else {
            if (other.LHS != NULL) *LHS = *other.LHS;
            else LHS = NULL;
            if (other.RHS != NULL) *RHS = *other.RHS;
            else RHS = NULL;
            atom_expr = NULL;
        }
        return *this;
    }
    void print_() const {
        if (isatom()) printf("is atom %d, %s\n", atom_expr->atom_value, atom_expr->atom_name.c_str());
        else printf("not atom\n");
    }
};

class let_AST{
public:
    unique_ptr<expr_AST> lvalue;
    unique_ptr<expr_AST> rvalue;
    let_AST(){}
    let_AST(unique_ptr<expr_AST> lvalue_, unique_ptr<expr_AST> rvalue_){
        if (lvalue_ != NULL) *lvalue = *lvalue_;
        if (rvalue_ != NULL) *rvalue = *rvalue_;
    }
    string lval(){return lvalue->name();}
    let_AST(const let_AST &rhs){
        if (rhs.lvalue != NULL) *lvalue = *rhs.lvalue;
        if (rhs.rvalue != NULL) *rvalue = *rhs.rvalue;
    }
    let_AST &operator=(const let_AST &rhs){
        if (&rhs == this) return *this;
        if (rhs.lvalue != NULL) *lvalue = *rhs.lvalue;
        else lvalue = NULL;
        if (rhs.rvalue != NULL) *rvalue = *rhs.rvalue;
        else rvalue = NULL;
        return *this;
    }
};

class input_AST{
    vector<expr_AST> identifiers;
public:
    input_AST(){}
    void push(unique_ptr<expr_AST> identifier){printf("push\n"); identifiers.push_back(*identifier);printf("push\n"); }
    int size(){return identifiers.size();}
    string fetch(int index){return identifiers[index].name();}
};

class exit_AST{
    unique_ptr<expr_AST> exit_expr;
public:
    exit_AST(){}
    exit_AST(unique_ptr<expr_AST> exit_expr_){
        if (exit_expr_ != NULL) *exit_expr = *exit_expr_;
    }
    exit_AST(const exit_AST &other){
        if (other.exit_expr != NULL) *exit_expr = *other.exit_expr;
    }
    exit_AST &operator=(const exit_AST &rhs){
        if (&rhs == this) return *this;
        if (rhs.exit_expr != NULL) *exit_expr = *rhs.exit_expr;
        else exit_expr = NULL;
        return *this;
    }
};

class goto_AST{
    int go_to;
public:
    goto_AST(int go_to_ = 0): go_to(go_to_){}
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
    stmt_AST(unique_ptr<let_AST> stmt_){
        if (stmt_ != NULL) *let_stmt = *stmt_;
        isrem_ = 0;
    }
    stmt_AST(unique_ptr<input_AST> stmt_){
        if (stmt_ != NULL) *input_stmt = *stmt_;
        isrem_ = 0;
    }
    stmt_AST(unique_ptr<exit_AST> stmt_){
        if (stmt_ != NULL) *exit_stmt = *stmt_;
        isrem_ = 0;
    }
    stmt_AST(unique_ptr<goto_AST> stmt_){
        if (stmt_ != NULL) *goto_stmt = *stmt_;
        isrem_ = 0;
    }
    stmt_AST(unique_ptr<if_AST> stmt_);// if_stmt(new if_AST(*stmt_)){isrem_ = 0;}
    stmt_AST(unique_ptr<for_AST> stmt_);// for_stmt(new for_AST(*stmt_)){isrem_ = 0;}
    stmt_AST(const stmt_AST &other);
    stmt_AST &operator=(const stmt_AST &rhs);
    bool isrem(){return isrem_;}
};

class if_AST{
public:
    unique_ptr<expr_AST> if_expr;
    unique_ptr<stmt_AST> if_stmt;
    if_AST(){}
    if_AST(unique_ptr<expr_AST> if_expr_, unique_ptr<stmt_AST> if_stmt_){
        if (if_expr_ != NULL) *if_expr = *if_expr_;
        if (if_stmt_ != NULL) *if_stmt = *if_stmt_;
    }
    if_AST(const if_AST &other){
        if (other.if_expr != NULL) *if_expr = *other.if_expr;
        if (other.if_stmt != NULL) *if_stmt = *other.if_stmt;
    }
    if_AST &operator=(const if_AST &rhs){
        if (&rhs == this) return *this;
        if (rhs.if_expr != NULL) *if_expr = *rhs.if_expr;
        else if_expr = NULL;
        if (rhs.if_stmt != NULL) *if_stmt = *rhs.if_stmt;
        else if_stmt = NULL;
        return *this;
    }
};

class for_AST{
public:
    vector<stmt_AST> stmts;
    unique_ptr<stmt_AST> it_stmt;
    unique_ptr<expr_AST> continue_gate;
    for_AST(unique_ptr<stmt_AST> it_stmt_, unique_ptr<expr_AST> continue_gate_){
        if (it_stmt_ != NULL) *it_stmt = *it_stmt_;
        if (continue_gate_ != NULL) *continue_gate = *continue_gate_;
    }
    void push(const stmt_AST &stmt_){stmts.push_back(stmt_);}
    for_AST(const for_AST &other){
        if (other.it_stmt != NULL) *it_stmt = *other.it_stmt;
        if (other.continue_gate != NULL) *continue_gate = *other.continue_gate;
    }
    for_AST &operator=(const for_AST &rhs){
        if (&rhs == this) return *this;
        if (rhs.it_stmt != NULL) *it_stmt = *rhs.it_stmt;
        else it_stmt = NULL;
        if (rhs.continue_gate != NULL) *continue_gate = *rhs.continue_gate;
        else continue_gate = NULL;
        return *this;
    }
};

stmt_AST::stmt_AST(unique_ptr<if_AST> stmt_){
    if (stmt_ != NULL) *if_stmt = *stmt_;
    isrem_ = 0;
}
stmt_AST::stmt_AST(unique_ptr<for_AST> stmt_){
    if (stmt_ != NULL) *for_stmt = *stmt_;
    isrem_ = 0;
}
stmt_AST::stmt_AST(const stmt_AST &other){
    isrem_ = other.isrem_;
    if (other.if_stmt != NULL) *if_stmt = *other.if_stmt;
    if (other.for_stmt != NULL) *for_stmt = *other.for_stmt;
    if (other.goto_stmt != NULL) *goto_stmt = *other.goto_stmt;
    if (other.exit_stmt != NULL) *exit_stmt = *other.exit_stmt;
    if (other.input_stmt != NULL) *input_stmt = *other.input_stmt;
    if (other.let_stmt != NULL) *let_stmt = *other.let_stmt;
}
stmt_AST &stmt_AST::operator=(const stmt_AST &other){
    if (&other == this) return *this;
    isrem_ = other.isrem_;
    if (other.if_stmt != NULL) *if_stmt = *other.if_stmt;
    if (other.for_stmt != NULL) *for_stmt = *other.for_stmt;
    if (other.goto_stmt != NULL) *goto_stmt = *other.goto_stmt;
    if (other.exit_stmt != NULL) *exit_stmt = *other.exit_stmt;
    if (other.input_stmt != NULL) *input_stmt = *other.input_stmt;
    if (other.let_stmt != NULL) *let_stmt = *other.let_stmt;
    return *this;
}

class program_AST{
    map<int, stmt_AST> stmts;
    vector<int> lines;
public:
    void push(int line_, const stmt_AST &stmt){stmts[line_] = stmt, lines.push_back(line_);}
};
#endif //BASIC_COMPILER_AST_H
