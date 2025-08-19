// binary_db.cpp
// 功能：使用二进制文件保存一系列结构化数据记录
// 支持：追加写、顺序读、按 ID 查找、按 ID 删除（逻辑删除）
// 文件格式：自定义头部（magic/版本/校验），记录头（id/时间戳/删除标志/载荷长度/载荷CRC），变长载荷（两个长度前缀字符串）
// CLI：add/list/get/del/help/quit
// 构建：见文末 CMakeLists.txt 示例（把此文件命名为 binary_db.cpp）

#include <cage.h>


int main(int argc, char* argv[]) {
    std::string dbpath = "records.bin";
    if (argc >= 2) dbpath = argv[1];
    std::filesystem::path path(dbpath);
    std::string pat = path.extension();
    if (pat == ".enc") {
    std::string decrypted = path.stem().string() + ".bin"; // 构造 .bin 文件名
    encryptbin(dbpath, "060305");                         // 解密
    dbpath = decrypted;                                   // 更新 dbpath                                        // 更新 path
    }
    BinaryDB db(dbpath);
    if (!db.open()) {
        std::cerr << "Failed to open DB file: " << dbpath << "\n";
        return 1;
    }

    std::cout << "BinaryDB opened: " << dbpath << "\n";
    print_help();

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        // 简易解析（空格分割，note 支持空格：从第三个token起拼回）
        std::istringstream iss(line);
        std::string cmd; iss >> cmd;
        if (cmd == "quit" || cmd == "exit") break;
        else if (cmd == "help") { print_help(); continue; }
        else if (cmd == "add") {
            uint32_t id; std::string name; std::string rest;
            if (!(iss >> id >> name)) { std::cout << "Usage: add <id> <name> <note...>\n"; continue; }
            std::getline(iss, rest); // 前导空格保留
            if (!rest.empty() && rest[0] == ' ') rest.erase(0,1);
            if (db.add(id, name, rest)) std::cout << "Added id=" << id << "\n"; else std::cout << "Add failed\n";
        }
        else if (cmd == "get") {
            uint32_t id; if (!(iss >> id)) { std::cout << "Usage: get <id>\n"; continue; }
            db.get(id);
        }
        else if (cmd == "list") {
            std::string opt; iss >> opt; bool all = (opt == "all");
            db.list(all);
        }
        else if (cmd == "del") {
            uint32_t id; if (!(iss >> id)) { std::cout << "Usage: del <id>\n"; continue; }
            db.del(id);
        }
        else {
            std::cout << "Unknown command: " << cmd << "\n";
            print_help();
        }
      
    
    }
    db.close();
    if (pat == ".enc")
    encryptenc(dbpath, "060305");
    std::cout<<path.extension() << " Database encrypted successfully.\n";
    return 0;
}

/* ---------------------- CMakeLists.txt 示例 ----------------------
cmake_minimum_required(VERSION 3.15)
project(binary_db)
set(CMAKE_CXX_STANDARD 17)
add_executable(binary_db binary_db.cpp)
# 使用：
#   mkdir build && cd build
#   cmake .. && cmake --build .
#   ./binary_db               # 使用默认文件 records.bin
#   ./binary_db mydata.bin    # 指定文件路径
*/
