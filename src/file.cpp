#include "../include/cage.h"


// ---------------------- 文件格式定义 ----------------------



// 载荷：两段字符串（name, note），均为 uint32_t 长度前缀 + 字节
 std::vector<uint8_t> encode_payload(const std::string& name, const std::string& note) {
    std::vector<uint8_t> buf;
    auto put_u32 = [&](uint32_t v) {
        for (int i = 0; i < 4; ++i) buf.push_back(uint8_t((v >> (i*8)) & 0xFF)); // 小端
    };
    put_u32(static_cast<uint32_t>(name.size()));
    buf.insert(buf.end(), name.begin(), name.end());
    put_u32(static_cast<uint32_t>(note.size()));
    buf.insert(buf.end(), note.begin(), note.end());
    return buf;
    
}

 bool decode_payload(const std::vector<uint8_t>& buf, std::string& name, std::string& note) {
    auto get_u32 = [&](size_t off, uint32_t& v)->bool{
        if (off + 4 > buf.size()) return false;
        v = (uint32_t)buf[off] | ((uint32_t)buf[off+1] << 8) | ((uint32_t)buf[off+2] << 16) | ((uint32_t)buf[off+3] << 24);
        return true;
    };
    size_t off = 0; uint32_t nlen = 0, mlen = 0;
    if (!get_u32(off, nlen)) return false; off += 4;
    if (off + nlen > buf.size()) return false;
    name.assign(reinterpret_cast<const char*>(&buf[off]), nlen); off += nlen;
    if (!get_u32(off, mlen)) return false; off += 4;
    if (off + mlen > buf.size()) return false;
    note.assign(reinterpret_cast<const char*>(&buf[off]), mlen); off += mlen;
    return off == buf.size();
}

