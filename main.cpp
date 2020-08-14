#include <bitset>
#include "Parser.h"

int main() {
    freopen("D:\\ClionProjects\\Basic Compiler\\testcases\\control_test\\control_1.txt", "r", stdin);
    freopen("D:\\ClionProjects\\RISCV\\test.txt", "w", stdout);
    binop_login();
    auto prog = main_parse();
    prog.generate_CFG();
    generate_code();
}