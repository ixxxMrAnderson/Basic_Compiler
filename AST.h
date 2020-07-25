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
    atom_AST(const atom_AST &other): atom_value(other.atom_value), atom_name(other.atom_name), atom_type(other.atom_type){}
    void generate(){
        if (atom_type == NUMBER) printf("%d", atom_value);
        else printf("%s", atom_name.c_str());
    }
};

class expr_AST{
public:
    BINOP binop;
    unique_ptr<atom_AST> atom_expr;
    unique_ptr<expr_AST> LHS, RHS;
    vector<unique_ptr<expr_AST>> indexes;
    expr_AST(){}
    expr_AST(BINOP op, unique_ptr<expr_AST> LHS_, unique_ptr<expr_AST> RHS_): binop(op), LHS(move(LHS_)), RHS(move(RHS_)){
        atom_expr = NULL;
    }
    expr_AST(unique_ptr<atom_AST> atom_expr_): atom_expr(move(atom_expr_)){
        LHS = NULL, RHS = NULL;
    }
//    expr_AST(expr_AST &other)
//            : binop(other.binop), atom_expr(move(other.atom_expr)), LHS(move(other.LHS)), RHS(move(other.RHS)){
//        printf("constructor\n");
//    }
    bool isatom() const{return atom_expr != NULL;}
    int value(){return atom_expr->atom_value;}
    string name(){return atom_expr->atom_name;}
    void push(unique_ptr<expr_AST> index_){indexes.push_back(move(index_));}
    void generate(){
        if (atom_expr != NULL){
            atom_expr->generate();
            for (int i = 0; i < indexes.size(); ++i){
                printf("[");
                indexes[i]->generate();
                printf("]");
            }
        }
        else {
            printf("(");
            LHS->generate();
            printf(" %s ", binop_str[binop]);
            RHS->generate();
            printf(")");
        }
    }
};

class let_AST{
public:
    unique_ptr<expr_AST> lvalue;
    unique_ptr<expr_AST> rvalue;
    let_AST(){}
    let_AST(unique_ptr<expr_AST> lvalue_, unique_ptr<expr_AST> rvalue_, unique_ptr<expr_AST> index_)
            : lvalue(move(lvalue_)), rvalue(move(rvalue_)){}
//    let_AST(let_AST &other)
//            :lvalue(move(other.lvalue)), rvalue(move(other.rvalue)){}
    void generate(){
        printf("LET_stmt ");
        lvalue->generate();
        printf(" = ");
        rvalue->generate();
        printf("\n");
    }
};

class input_AST{
    vector<unique_ptr<expr_AST>> identifiers;
public:
    input_AST(): identifiers(0){}
    void push(unique_ptr<expr_AST> identifier){identifiers.push_back(move(identifier));}
    void generate(){
        printf("INPUT_stmt ");
        for (int i = 0; i < identifiers.size(); ++i){
            printf("%s", identifiers[i]->name().c_str());
            if (i == identifiers.size() - 1) printf("\n");
            else printf(", ");
        }
    }
};

class exit_AST{
    unique_ptr<expr_AST> exit_expr;
public:
    exit_AST(){}
    exit_AST(unique_ptr<expr_AST> exit_expr_)
            : exit_expr(move(exit_expr_)){}
//    exit_AST(exit_AST &other)
//            : exit_expr(move(other.exit_expr)){}
    void generate(){
        printf("EXIT_stmt ");
        exit_expr->generate();
        printf("\n");
    }
};

class goto_AST{
    int go_to;
public:
    goto_AST(int go_to_ = 0): go_to(go_to_){}
    void generate(){printf("GOTO_stmt %d\n", go_to);}
};

class if_AST;
class for_AST;
class stmt_AST{
public:
    unique_ptr<let_AST> let_stmt = NULL;
    unique_ptr<input_AST> input_stmt = NULL;
    unique_ptr<exit_AST> exit_stmt = NULL;
    unique_ptr<goto_AST> goto_stmt = NULL;
    unique_ptr<if_AST> if_stmt = NULL;
    unique_ptr<for_AST> for_stmt = NULL;
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
//    stmt_AST(stmt_AST &other);
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
//    if_AST(if_AST &other)
//            :if_goto(move(other.if_goto)), if_expr(move(other.if_expr)){}
    void generate(){
        printf("IF_stmt ");
        if_expr->generate();
        printf(" THEN ");
        if_goto->generate();
        printf("\n");
    }
};

class for_AST{
public:
    map<int, unique_ptr<stmt_AST>> stmts;
    vector<int> lines;
    unique_ptr<stmt_AST> it_stmt = NULL;
    unique_ptr<expr_AST> continue_gate = NULL;
    for_AST(){}
    for_AST(unique_ptr<stmt_AST> it_stmt_, unique_ptr<expr_AST> continue_gate_)
            :it_stmt(move(it_stmt_)), continue_gate(move(continue_gate_)){}
    void push(int line, unique_ptr<stmt_AST> stmt){
//        printf("inside for: %d\n", line);
        stmts[line] = move(stmt);
        lines.push_back(line);
    }
//    for_AST(for_AST &other)
//            :it_stmt(move(other.it_stmt)), continue_gate(move(other.continue_gate)){}
    void generate(){
        printf("FOR_stmt ");
        it_stmt->generate();
        printf("  gate: ");
        continue_gate->generate();
        printf("\n");
        for (int i = 0; i < lines.size(); ++i){
            printf("  line %d: ", lines[i]);
            stmts[lines[i]]->generate();
        }
        printf("END FOR\n");
    }
};

stmt_AST::stmt_AST(unique_ptr<if_AST> stmt_)
            : if_stmt(move(stmt_)){isrem_ = 0;}
stmt_AST::stmt_AST(unique_ptr<for_AST> stmt_)
            : for_stmt(move(stmt_)){isrem_ = 0;}
//stmt_AST::stmt_AST(stmt_AST &other)
//: if_stmt(move(other.if_stmt)), for_stmt(move(other.for_stmt)), goto_stmt(move(other.goto_stmt))
//, exit_stmt(move(other.exit_stmt)), input_stmt(move(other.input_stmt)), let_stmt(move(other.let_stmt)){isrem_ = other.isrem_;}
void stmt_AST::generate(){
    if (let_stmt != NULL) return let_stmt->generate();
    if (input_stmt != NULL) return input_stmt->generate();
    if (exit_stmt != NULL) return exit_stmt->generate();
    if (goto_stmt != NULL) return goto_stmt->generate();
    if (if_stmt != NULL) return if_stmt->generate();
    if (for_stmt != NULL) return for_stmt->generate();
}

class program_AST{
    map<int, unique_ptr<stmt_AST>> stmts;
    vector<int> lines;
public:
    void push(int line_, unique_ptr<stmt_AST> stmt){stmts[line_] = move(stmt), lines.push_back(line_);}
    void generate(){
        for (int i = 0 ; i < lines.size(); ++i){
            printf("line %d: ", lines[i]);
            stmts[lines[i]]->generate();
        }
    }
};
#endif //BASIC_COMPILER_AST_H
