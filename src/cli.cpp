#include "../include/cage.h"


const char* MAGIC = "BINDATA"; // 实际占 7 字节 + 1 NUL
bool crc32_init_done = false;
uint32_t crc32_table[256];

void print_help() {
    std::cout << "Commands:\n"
                 "  add <id> <name> <note...>    : 追加一条记录\n"
                 "  get <id>                     : 按ID查找记录\n"
                 "  list [all]                  : 顺序列出记录（加 all 显示删除的）\n"
                 "  del <id>                    : 按ID逻辑删除\n"
                 "  help                        : 帮助\n"
                 "  quit/exit                   : 退出\n";
}