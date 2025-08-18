#include <fstream>
#include <iostream>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    std::string  in = "records.enc", out = "smile.bin", key="060305";
    std::ifstream fin(in, std::ios::binary);
    if (!fin) { std::cerr << "无法打开输入: " << in << "\n"; return 1; }
    std::ofstream fout(out, std::ios::binary);
    if (!fout) { std::cerr << "无法打开输出: " << out << "\n"; return 1; }
    std::vector<char> buf(1<<20); // 1MB 缓冲
    size_t ki = 0, klen = key.size();
    while (fin) {
        fin.read(buf.data(), buf.size());
        std::streamsize got = fin.gcount();
        for (std::streamsize i = 0; i < got; ++i) {
            buf[i] ^= key[ki];
            ki = (ki + 1) % klen;
        }
        fout.write(buf.data(), got);
    }
    std::cout << "完成解码" << "\n";
    return 0;
}