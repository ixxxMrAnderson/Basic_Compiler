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

unique_ptr<expr_AST> expr_parse();
unique_ptr<expr_AST> number_parse(){
    auto result = make_unique<atom_AST>(number_token, "!");
    get_next_token();
    return make_unique<expr_AST>(move(result));
}

unique_ptr<expr_AST> identifier_parse(){
    auto result = make_unique<atom_AST>(0, identifier_token);
    get_next_token();
    return make_unique<expr_AST>(move(result));
}

unique_ptr<expr_AST> paren_parse(){
    int tmp_token = cur_token;
    get_next_token();
    auto contents = expr_parse();
    if (!contents) return nullptr;
    if (tmp_token == '(' && cur_token != ')') log_error("expected ')'");
    if (tmp_token == '[' && cur_token != ']') log_error("expected ']'");
    get_next_token();
    return contents;
}

unique_ptr<expr_AST> primary_parse(){
    auto return_ = make_unique<expr_AST>();
    switch (cur_token){
        case number_: case line_:
            return number_parse();
        case identifier_:
            return_ =  identifier_parse();
            break;
        case '(':
            return paren_parse();
        case LET_: case INPUT_: case GOTO_: case FOR_: case IF_: case THEN_: case EXIT_: case END_: case REM_:
            log_error("unexpected statement declaration");
            break;
        default:
            return_ = nullptr;
    }
    while (cur_token == '[') return_->push(move(paren_parse()));
    //if (return_->indexes.size()) printf("primary_parse: %s\n", return_->generate_str().c_str());
    return return_;
}

map<BINOP, int> binop_precedence;
void binop_login(){
    binop_precedence[OR] = 20;
    binop_precedence[AND] = 40;
    binop_precedence[EQ] = 60;
    binop_precedence[NOT_EQ] = 60;
    binop_precedence[SMALLER] = 80;
    binop_precedence[GREATER] = 80;
    binop_precedence[SMALLER_EQ] = 80;
    binop_precedence[GREATER_EQ] = 80;
    binop_precedence[ADD] = 100;
    binop_precedence[SUB] = 100;
    binop_precedence[DIVIDE] = 120;
    binop_precedence[TIMES] = 120;
    binop_precedence[MOD] = 120;
}

int get_precedence(){
    if (cur_token != binop_) return -1;
    int prec = binop_precedence[binop_token];
    return prec;
}

unique_ptr<expr_AST> binop_RHS_parse(int expr_prec, unique_ptr<expr_AST> LHS){
    while (1){
        int cur_prec = get_precedence();
        if (cur_prec < expr_prec) return LHS;
        BINOP binop = binop_token;
        get_next_token(); // eat binop
        auto RHS = primary_parse();
        if (!RHS) return nullptr;
        int next_prec = get_precedence();
        if (cur_prec < next_prec){
            RHS = binop_RHS_parse(cur_prec + 1, move(RHS));
            if (!RHS) return nullptr;
        }
        LHS = make_unique<expr_AST>(binop, move(LHS), move(RHS));
    }
}

unique_ptr<expr_AST> expr_parse(){
    auto LHS = primary_parse();
    if (!LHS) return nullptr;
    return binop_RHS_parse(0, move(LHS));
}

unique_ptr<let_AST> let_parse(){
    if (cur_token == LET_) get_next_token(); //Eat LET.
    auto lval = expr_parse();
    unique_ptr<expr_AST> index = nullptr;
    if (cur_token != '=') log_error("LET statement should be followed by an assignment");
    get_next_token(); //Eat '='.
    auto rexpr = expr_parse();
    if (cur_token != line_ && cur_token != ';') log_error("2 assignments in LET statement");
    return make_unique<let_AST>(move(lval), move(rexpr), move(index));
}

unique_ptr<input_AST> input_parse(){
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
    get_next_token(); //Eat GOTO.
    auto goto_ = primary_parse()->value();
    if (!goto_) log_error("GOTO statement must be followed by a list of constant integers");
    return make_unique<goto_AST>(goto_);
}

unique_ptr<exit_AST> exit_parse(){
    get_next_token(); //Eat EXIT.
    if (cur_token != number_ && cur_token != identifier_ && cur_token != '('){
        log_error("exit");
    }
    return make_unique<exit_AST>(move(expr_parse()));
}

unique_ptr<stmt_AST> stmt_parse(int line__);
unique_ptr<if_AST> if_parse(){
    get_next_token(); //Eat IF.
    auto if_expr = expr_parse();
    get_next_token(); //Eat THEN.
    if (cur_token != number_) log_error("short circuit evaluation");
    cout << number_token << endl;
    unique_ptr<expr_AST> if_goto(move(number_parse()));
    if (cur_token != line_) cout << cur_token << endl, cout << number_token << endl;
    return make_unique<if_AST>(move(if_expr), move(if_goto));
}

int cnt_for_line = 1;
unique_ptr<for_AST> for_parse(int for_line_){
    for_lines["FOR_" + to_string(for_line_)] = cnt_for_line++;
    get_next_token(); //Eat FOR.
    unique_ptr<stmt_AST> it_stmt(new stmt_AST(let_parse()));
    get_next_token(); //Eat ';'.
    int line;
    auto continue_gate = expr_parse();
    auto tmp_for = make_unique<for_AST>(move(it_stmt), move(continue_gate), for_line_);
    while (cur_token != END_) {
        if (cur_token != line_) cout << cur_token << endl;
        line = primary_parse()->value();
        if (!line) log_error("Line number must be a constant integer");
        if (cur_token == END_) break;
        tmp_for->push(line, stmt_parse(line));
    }
    tmp_for->end_for = line;
    get_next_token(); //Eat END.
    tmp_for->after_end_for = number_token;
    return tmp_for;
}

unique_ptr<stmt_AST> rem_parse(){
    get_next_token();
    return unique_ptr<stmt_AST>(new stmt_AST());
}

unique_ptr<stmt_AST> stmt_parse(int line__){
    switch (cur_token) {
        case LET_:
            return make_unique<stmt_AST>(let_parse());
        case INPUT_:
            return make_unique<stmt_AST>(input_parse());
        case GOTO_:
            return make_unique<stmt_AST>(goto_parse());
        case EXIT_:
            return make_unique<stmt_AST>(exit_parse());
        case IF_:
            return make_unique<stmt_AST>(if_parse());
        case FOR_:
            return make_unique<stmt_AST>(for_parse(line__));
        case REM_:
            return rem_parse();
        default:
            log_error("expected declaration of statement");
            return nullptr;
    }
}

program_AST main_parse(){
    program_AST prog;
    int line__;
    get_next_token();
    while (1){
        auto line = primary_parse();
        if (!line) return prog;
        else line__ = line->value();
        if (!line) log_error("Line number must be a constant integer");
        if (cur_token == EOF_) return prog;
        else {
            auto st_ = stmt_parse(line__);
            prog.push(line__, move(st_));
        }
        if (cur_token != line_) printf("?\n");
    }
}

#endif //BASIC_COMPILER_PARSER_H
