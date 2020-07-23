//
// Created by 储浩天 on 2020/7/16.
//

#ifndef BASIC_COMPILER_PARSER_H
#define BASIC_COMPILER_PARSER_H

#include "Lexer.h"

int cur_token;
int get_next_token(){
    return cur_token = get_token();
}

unique_ptr<expr_AST> log_error(const char *str){
    printf("log_error: %s\n", str);
    return nullptr;
}

unique_ptr<expr_AST> expr_parse();
unique_ptr<expr_AST> number_parse(){
    printf("Parsed a number atom: %d\n", number_token);
    auto result = make_unique<atom_AST>(number_token, "!");
    get_next_token();
    return make_unique<expr_AST>(move(result));
}

unique_ptr<expr_AST> identifier_parse(){
    printf("Parsed a identifier atom: %s\n", identifier_token.c_str());
    auto result = make_unique<atom_AST>(0, identifier_token);
    get_next_token();
    return make_unique<expr_AST>(move(result));
}

unique_ptr<expr_AST> paren_parse(){
    get_next_token(); // Eat '('.
    auto contents = expr_parse();
    if (!contents) return nullptr;
    if (cur_token != ')') return log_error("expected ')'");
    get_next_token(); // Eat ')'.
    printf("Parsed a paren expr\n");
    return contents;
}

//handle atom and paren unit
unique_ptr<expr_AST> primary_parse(){
    switch (cur_token){
        default:
            return log_error("unknown token when expecting an expression");
        case number_: case line_:
            return number_parse();
        case identifier_:
            return identifier_parse();
        case '(':
            return paren_parse();
    }
}

map<BINOP, int> binop_precedence; // this holds the precedence for each binary operator that is defined.
int get_precedence(){
    if (cur_token != binop_) return -1;
    int prec = binop_precedence[binop_token];
    return prec;
}

unique_ptr<expr_AST> binop_RHS_parse(int expr_prec, unique_ptr<expr_AST> LHS){
    while (1){
        int cur_prec = get_precedence();
        if (cur_prec < expr_prec) {
            //printf("cur binop pre is lower(login: %d, cur: %d)\n", expr_prec, cur_prec);
            return LHS;
        }
        BINOP binop = binop_token;
        get_next_token(); // eat binop
        auto RHS = primary_parse();
        if (!RHS) {
            //printf("RHS is empty(login: %d, cur: %d)\n", expr_prec, cur_prec);
            return nullptr;
        }
        int next_prec = get_precedence();
        if (cur_prec < next_prec){
            //printf("calculate RHS first(login: %d, cur: %d)\n", expr_prec, cur_prec);
            RHS = binop_RHS_parse(cur_prec + 1, move(RHS));
            if (!RHS) return nullptr;
        }
        //printf("Merging LHS & RHS(login: %d, cur: %d)\n", expr_prec, cur_prec);
        LHS = make_unique<expr_AST>(binop, move(LHS), move(RHS));
    }
}

unique_ptr<expr_AST> expr_parse(){
    auto LHS = primary_parse();
    if (!LHS) return nullptr;
    return binop_RHS_parse(0, move(LHS));
}

//handle exprssion unit
//void top_expr_parse(){
//    if (expr_parse()) printf("Parsed a top-level expr\n");
//    else get_next_token();
//}

unique_ptr<let_AST> let_parse(){
    printf("LET_parse\n");
    if (cur_token == LET_) get_next_token(); //Eat LET.
    auto lval = primary_parse();
    get_next_token(); //Eat '='.
    auto rexpr = expr_parse();
    return make_unique<let_AST>(move(lval), move(rexpr));
}

unique_ptr<input_AST> input_parse(){
    printf("INPUT_parse\n");
    get_next_token(); //Eat INPUT.
    unique_ptr<input_AST> tmp_input(new input_AST);
    tmp_input->push(move(expr_parse()));
    while(cur_token == ','){
        get_next_token();
        tmp_input->push(move(expr_parse()));
    }
    return tmp_input;
}

unique_ptr<goto_AST> goto_parse(){
    printf("GOTO_parse\n");
    get_next_token(); //Eat GOTO.
    return make_unique<goto_AST>(primary_parse()->value());
}

unique_ptr<exit_AST> exit_parse(){
    printf("EXIT_parse\n");
    get_next_token(); //Eat EXIT.
    return make_unique<exit_AST>(move(expr_parse()));
}

unique_ptr<stmt_AST> stmt_parse();
unique_ptr<if_AST> if_parse(){
    printf("IF_parse\n");
    get_next_token(); //Eat IF.
    auto if_expr = expr_parse();
    printf("THEN, %d", cur_token);
    get_next_token(); //Eat THEN.
    unique_ptr<expr_AST> if_goto(move(primary_parse()));
    return make_unique<if_AST>(move(if_expr), move(if_goto));
}

unique_ptr<for_AST> for_parse(){
    printf("FOR_parse\n");
    get_next_token(); //Eat FOR.
    unique_ptr<stmt_AST> it_stmt(new stmt_AST(let_parse()));
    get_next_token(); //Eat ';'.
    auto continue_gate = expr_parse();
    auto tmp_for = make_unique<for_AST>(move(it_stmt), move(continue_gate));
    while (cur_token != END_) {
        printf("pushing stmt: %d\n", cur_token);
        int line = primary_parse()->value();
        if (cur_token == END_) break;
        tmp_for->push(line, stmt_parse());
    }
    get_next_token(); //Eat END.
    return tmp_for;
}

unique_ptr<stmt_AST> rem_parse(){
    get_next_token();
    return unique_ptr<stmt_AST>(new stmt_AST());
}

unique_ptr<stmt_AST> stmt_parse(){
    switch (cur_token) {
        case INPUT_:
            return make_unique<stmt_AST>(input_parse());
        case GOTO_:
            return make_unique<stmt_AST>(goto_parse());
        case EXIT_:
            return make_unique<stmt_AST>(exit_parse());
        case IF_:
            return make_unique<stmt_AST>(if_parse());
        case FOR_:
            return make_unique<stmt_AST>(for_parse());
        case REM_:
            return rem_parse();
        default:
            return make_unique<stmt_AST>(let_parse());
    }
}

program_AST main_parse(){
    program_AST prog;
    int line_;
    while (1){
        printf("ready>\n");
        auto line = primary_parse();
        if (line == NULL) return prog;
        else line_ = line->value();
        if (cur_token == EOF_) return prog;
        else {
            auto st_ = stmt_parse();
            if (!st_->isrem()) prog.push(line_, move(st_));
        }
        printf("<loop over\n");
    }
}

#endif //BASIC_COMPILER_PARSER_H
