#include <iostream>
#include "Parser.h"

int main() {
    freopen("D:\\ClionProjects\\Basic Compiler\\testcases\\op_test\\op_10.txt", "r", stdin);
    binop_login();
    get_next_token();
    program_AST prog = main_parse();
    printf("------------------------------------------\n");
    prog.generate();
}
