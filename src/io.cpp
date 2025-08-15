#include "../include/cage.h"



// ---------------------- I/O 辅助 ----------------------
uint64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

bool write_file_header(std::fstream& f) {
    FileHeader h{};
    std::memset(h.magic, 0, sizeof(h.magic));
    std::strncpy(h.magic, MAGIC, sizeof(h.magic)-1);
    h.version = VERSION;
    h.header_crc = 0;
    // 计算 CRC（不含 header_crc 本身）
    uint8_t tmp[8 + 4];
    std::memset(tmp, 0, sizeof(tmp));
    std::memcpy(tmp, h.magic, 8);
    std::memcpy(tmp + 8, &h.version, 4);
    h.header_crc = crc32(tmp, sizeof(tmp));

    f.seekp(0, std::ios::beg);
    f.write(reinterpret_cast<const char*>(&h), sizeof(h));
    return f.good();
}

bool read_file_header(std::fstream& f, FileHeader& out) {
    f.seekg(0, std::ios::beg);
    if (!f.read(reinterpret_cast<char*>(&out), sizeof(out))) return false;
    // 校验 magic
    if (std::strncmp(out.magic, MAGIC, std::strlen(MAGIC)) != 0) return false;
    // 校验 CRC
    uint8_t tmp[8 + 4];
    std::memset(tmp, 0, sizeof(tmp));
    std::memcpy(tmp, out.magic, 8);
    std::memcpy(tmp + 8, &out.version, 4);
    uint32_t c = crc32(tmp, sizeof(tmp));
    return c == out.header_crc;
}

std::streampos file_end(std::fstream& f) {
    f.seekg(0, std::ios::end);
    return f.tellg();
}