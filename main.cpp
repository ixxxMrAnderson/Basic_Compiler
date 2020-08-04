#include <bitset>
#include "Parser.h"

int main() {
    freopen("D:\\ClionProjects\\Basic Compiler\\testcases\\control_test\\control_1.txt", "r", stdin);
    binop_login();
    auto prog = main_parse();
    freopen("CON", "r", stdin);
    prog.generate_CFG();
    print_CFG();
    printf("-------------------\n");
    generate_code();
}