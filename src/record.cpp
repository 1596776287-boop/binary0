#include "../include/cage.h"

// ---------------------- 记录级操作 ----------------------


 bool write_record(std::fstream& f, const Record& r) {
    f.seekp(0, std::ios::end);
    f.write(reinterpret_cast<const char*>(&r.hdr), sizeof(r.hdr));
    if (!r.payload.empty())
        f.write(reinterpret_cast<const char*>(r.payload.data()), static_cast<std::streamsize>(r.payload.size()));
    return f.good();
}

 bool read_record_at(std::fstream& f, std::streampos pos, Record& r) {
    f.seekg(pos, std::ios::beg);
    if (!f.read(reinterpret_cast<char*>(&r.hdr), sizeof(r.hdr))) return false;
    r.payload.resize(r.hdr.payload_size);
    if (r.hdr.payload_size > 0) {
        if (!f.read(reinterpret_cast<char*>(r.payload.data()), r.hdr.payload_size)) return false;
    }
    // 校验 payload CRC
    if (crc32(r.payload.data(), r.payload.size()) != r.hdr.payload_crc) {
        std::cerr << "[WARN] Payload CRC mismatch at pos=" << pos << " (id=" << r.hdr.id << ")\n";
    }
    return true;
}

 bool overwrite_deleted_flag(std::fstream& f, std::streampos pos, uint8_t deleted) {
    // pos 指向记录头起始，deleted 在 RecordHeader 内第：offset = offsetof(RecordHeader, deleted)
    const std::streamoff off = static_cast<std::streamoff>(offsetof(RecordHeader, deleted));
    f.seekp(pos + off, std::ios::beg);
    f.write(reinterpret_cast<const char*>(&deleted), 1);
    return f.good();
}

// 顺序遍历：返回所有记录位置（便于列表/查找）
 std::vector<std::streampos> enumerate_record_positions(std::fstream& f) {
    std::vector<std::streampos> positions;
    f.seekg(0, std::ios::end);
    auto end = f.tellg();
    std::streampos pos = sizeof(FileHeader);
    while (pos < end) {
        f.seekg(pos, std::ios::beg);
        RecordHeader hdr{};
        if (!f.read(reinterpret_cast<char*>(&hdr), sizeof(hdr))) break;
        positions.push_back(pos);
        pos += static_cast<std::streamoff>(sizeof(RecordHeader)) + hdr.payload_size;
    }
    return positions;
}
