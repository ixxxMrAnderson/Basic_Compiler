#include <bitset>
#include "Parser.h"

int main() {
    freopen("D:\\ClionProjects\\Basic Compiler\\testcases\\op_test\\op_10.txt", "r", stdin);
    binop_login();
    auto prog = main_parse();
    freopen("CON", "r", stdin);
    prog.generate_CFG();
//    print_dump();
//    printf("-----------------------------------------------\n");
    generate_code();
}