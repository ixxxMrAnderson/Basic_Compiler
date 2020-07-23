#include <iostream>
#include "Parser.h"

int main() {
    freopen("D:\\ClionProjects\\Basic Compiler\\testcases\\basic_test\\basic_4.txt", "r", stdin);
    get_next_token();
    program_AST prog = main_parse();
    prog.generate();
}