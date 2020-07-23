#include <iostream>
#include "Parser.h"

int main() {
    freopen("D:\\ClionProjects\\Basic Compiler\\testcases\\op_test\\op_1.txt", "r", stdin);
    get_next_token();
    program_AST prog = main_parse();
    printf("------------------------------------------\n");
    prog.generate();
}