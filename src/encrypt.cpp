#include "../include/cage.h"

void encrypt(std::string in, std::string out, std::string key) {
    std::ifstream fin(in, std::ios::binary);
    std::ofstream fout(out, std::ios::binary);
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
    std::cout << "完成加密\n";
    fin.close();
    fout.close();
}