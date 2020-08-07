#include <bitset>
#include "Parser.h"

int main() {
    freopen("D:\\ClionProjects\\Basic Compiler\\testcases\\array_test\\array_5.txt", "r", stdin);
    binop_login();
    auto prog = main_parse();
    freopen("CON", "r", stdin);
    prog.generate_CFG();
    printf("-----------------------------------------------\n");
    print_dump();
    printf("-----------------------------------------------\n");
    generate_code();
    printf("-----------------------------------------------\nval2mem_\n");
    for (auto i = val2mem.begin(); i != val2mem.end(); ++i){
        printf("%s___%d\n", i->first.c_str(), i->second);
    }
}