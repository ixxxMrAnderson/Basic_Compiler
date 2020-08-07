#include <bitset>
#include "Parser.h"

int main() {
    freopen("D:\\ClionProjects\\Basic Compiler\\testcases\\op_test\\op_2.txt", "r", stdin);
    binop_login();
    auto prog = main_parse();
    freopen("CON", "r", stdin);
    prog.generate_CFG();
    printf("-----------------------------------------------\n");
    print_dump();
    printf("-----------------------------------------------\n");
    generate_code();
    printf("-----------------------------------------------\n");
    for (auto i = val2mem.begin(); i != val2mem.end(); ++i){
        printf("%s___%d\n", i->first.c_str(), i->second);
    }
}