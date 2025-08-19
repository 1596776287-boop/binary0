#include "../include/cage.h"

void encryptenc(const std::string& in, const std::string& key) {
    // 使用 filesystem 替换后缀为 .enc
    std::filesystem::path inPath(in);
    std::filesystem::path outPath = inPath;
    outPath.replace_extension(".enc"); // 替换后缀

    std::ifstream fin(in, std::ios::binary);
    if (!fin) {
        std::cerr << "无法打开输入文件: " << in << "\n";
        return;
    }

    std::ofstream fout(outPath, std::ios::binary);
    if (!fout) {
        std::cerr << "无法创建输出文件: " << outPath << "\n";
        return;
    }

    std::vector<char> buf(1 << 20); // 1MB 缓冲
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

    fin.close();
    fout.close();

    // 删除原文件
    std::filesystem::remove(in);
}
void encryptbin(const std::string& in, const std::string& key) {
    // 使用 filesystem 替换后缀为 .enc
    std::filesystem::path inPath(in);
    std::filesystem::path outPath = inPath;
    outPath.replace_extension(".bin"); // 替换后缀

    std::ifstream fin(in, std::ios::binary);
    if (!fin) {
        std::cerr << "无法打开输入文件: " << in << "\n";
        return;
    }

    std::ofstream fout(outPath, std::ios::binary);
    if (!fout) {
        std::cerr << "无法创建输出文件: " << outPath << "\n";
        return;
    }

    std::vector<char> buf(1 << 20); // 1MB 缓冲
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

    fin.close();
    fout.close();

    // 删除原文件
    std::filesystem::remove(in);
}

