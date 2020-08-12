#ifndef BASIC_COMPILER_LEXER_H
#define BASIC_COMPILER_LEXER_H

#include "AST.h"

long long number_token;
string identifier_token;
BINOP binop_token;
bool jd_start = 1;
int get_token(){
    int tmp_char = ' ';
    string tmp_str;
    while (isspace(tmp_char)){
        if (tmp_char == '\n') jd_start = 1;
        tmp_char = getchar();
    }
    if (jd_start){
        jd_start = 0;
        while(!isdigit(tmp_char) && tmp_char != EOF) tmp_char = getchar();
        if (tmp_char == EOF) return EOF_;
        string num_str;
        while (isdigit(tmp_char)){
            num_str += tmp_char;
            tmp_char = getchar();
        }
        cin.putback(tmp_char);
        number_token = strtod(num_str.c_str(), 0);
        return line_;
    } else if (isalpha(tmp_char)){
        tmp_str = tmp_char;
        while (isalpha(tmp_char = getchar())) tmp_str += tmp_char;
        cin.putback(tmp_char);
        if (tmp_str == "REM"){
            do tmp_char = getchar();
            while (tmp_char != EOF && tmp_char != '\n' && tmp_char != '\r');
            if (tmp_char != EOF) {cin.putback(tmp_char); return REM_;}
            else return EOF_;
        }
        else if (tmp_str == "LET") return LET_;
        else if (tmp_str == "INPUT") return INPUT_;
        else if (tmp_str == "EXIT") return EXIT_;
        else if (tmp_str == "GOTO") return GOTO_;
        else if (tmp_str == "IF") return IF_;
        else if (tmp_str == "THEN") return THEN_;
        else if (tmp_str == "FOR") return FOR_;
        else if (tmp_str == "END"){
            while (isspace(tmp_char) && tmp_char != '\n') tmp_char = getchar();
            if (tmp_char != 'F') printf("END&FOR not match\n");
            else getchar(), getchar();
            return END_;
        } else {
            identifier_token = tmp_str;
            return identifier_;
        }
    } else if (isdigit(tmp_char)){
        string num_str;
        do {
            num_str += tmp_char;
            tmp_char = getchar();
        } while (isdigit(tmp_char));
        cin.putback(tmp_char);
        number_token = strtod(num_str.c_str(), 0);
        return number_;
    } else if (tmp_char == EOF) return EOF_;
    else if (tmp_char == '+'){binop_token = ADD; return binop_;}
    else if (tmp_char == '-'){
        tmp_char = getchar();
        if (isdigit(tmp_char)){
            string num_str = "-";
            do {
                num_str += tmp_char;
                tmp_char = getchar();
            } while (isdigit(tmp_char));
            cin.putback(tmp_char);
            number_token = strtod(num_str.c_str(), 0);
            return number_;
        } else {
            cin.putback(tmp_char);
            binop_token = SUB;
            return binop_;
        }
    } else if (tmp_char == '*'){binop_token = TIMES; return binop_;}
    else if (tmp_char == '/'){binop_token = DIVIDE; return binop_;}
    else if (tmp_char == '%'){binop_token = MOD; return binop_;}
    else if (tmp_char == '<'){
        if ((tmp_char = getchar()) == '=') binop_token = SMALLER_EQ;
        else binop_token = SMALLER, cin.putback(tmp_char);
        return binop_;
    } else if (tmp_char == '>'){
        if ((tmp_char = getchar()) == '=') binop_token = GREATER_EQ;
        else binop_token = GREATER, cin.putback(tmp_char);
        return binop_;
    } else if (tmp_char == '='){
        if ((tmp_char = getchar()) == '='){binop_token = EQ; return binop_;}
        else {cin.putback(tmp_char); return int('=');}
    } else if (tmp_char == '!'){
        if ((tmp_char = getchar()) == '='){binop_token = NOT_EQ; return binop_;}
        else {cin.putback(tmp_char); return int('!');}
    } else if (tmp_char == '&'){
        if ((tmp_char = getchar()) == '&'){binop_token = AND; return binop_;}
        else {cin.putback(tmp_char); return int('&');}
    } else if (tmp_char == '|'){
        if ((tmp_char = getchar()) == '|'){binop_token = OR; return binop_;}
        else {cin.putback(tmp_char); return int('|');}
    } else return int(tmp_char);
    return 0;
}

#endif //BASIC_COMPILER_LEXER_H