// binary_db.cpp
// 功能：使用二进制文件保存一系列结构化数据记录
// 支持：追加写、顺序读、按 ID 查找、按 ID 删除（逻辑删除）
// 文件格式：自定义头部（magic/版本/校验），记录头（id/时间戳/删除标志/载荷长度/载荷CRC），变长载荷（两个长度前缀字符串）
// CLI：add/list/get/del/help/quit
// 构建：见文末 CMakeLists.txt 示例（把此文件命名为 binary_db.cpp）

#include <cage.h>





// ---------------------- CRC32（标准多项式 0xEDB88320） ----------------------
// static uint32_t crc32_table[256];
// static bool crc32_init_done = false;

// static void init_crc32() {
//     if (crc32_init_done) return;
//     for (uint32_t i = 0; i < 256; ++i) {
//         uint32_t c = i;
//         for (size_t j = 0; j < 8; ++j) {
//             c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
//         }
//         crc32_table[i] = c;
//     }
//     crc32_init_done = true;
// }

// static uint32_t crc32(const uint8_t* data, size_t len, uint32_t seed = 0xFFFFFFFFu) {
//     init_crc32();
//     uint32_t c = seed;
//     for (size_t i = 0; i < len; ++i) {
//         c = crc32_table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
//     }
//     return c ^ 0xFFFFFFFFu;
// }

// // ---------------------- 文件格式定义 ----------------------
// #pragma pack(push, 1)
// struct FileHeader {
//     char     magic[8];       // "BINDATA" + NUL（或类似标识）
//     uint32_t version;        // 版本号
//     uint32_t header_crc;     // 头部校验（覆盖 magic 和 version，不含本字段）
// };

// struct RecordHeader {
//     uint32_t id;             // 记录ID
//     uint64_t timestamp;      // 记录时间（epoch毫秒）
//     uint8_t  deleted;        // 0=有效, 1=逻辑删除
//     uint32_t payload_size;   // 载荷字节数
//     uint32_t payload_crc;    // 只对载荷做CRC（便于修改deleted而不破坏校验）
// };
// #pragma pack(pop)

// static const char* MAGIC = "BINDATA"; // 实际占 7 字节 + 1 NUL
// static const uint32_t VERSION = 1;

// // 载荷：两段字符串（name, note），均为 uint32_t 长度前缀 + 字节
// static std::vector<uint8_t> encode_payload(const std::string& name, const std::string& note) {
//     std::vector<uint8_t> buf;
//     auto put_u32 = [&](uint32_t v) {
//         for (int i = 0; i < 4; ++i) buf.push_back(uint8_t((v >> (i*8)) & 0xFF)); // 小端
//     };
//     put_u32(static_cast<uint32_t>(name.size()));
//     buf.insert(buf.end(), name.begin(), name.end());
//     put_u32(static_cast<uint32_t>(note.size()));
//     buf.insert(buf.end(), note.begin(), note.end());
//     return buf;
    
// }

// static bool decode_payload(const std::vector<uint8_t>& buf, std::string& name, std::string& note) {
//     auto get_u32 = [&](size_t off, uint32_t& v)->bool{
//         if (off + 4 > buf.size()) return false;
//         v = (uint32_t)buf[off] | ((uint32_t)buf[off+1] << 8) | ((uint32_t)buf[off+2] << 16) | ((uint32_t)buf[off+3] << 24);
//         return true;
//     };
//     size_t off = 0; uint32_t nlen = 0, mlen = 0;
//     if (!get_u32(off, nlen)) return false; off += 4;
//     if (off + nlen > buf.size()) return false;
//     name.assign(reinterpret_cast<const char*>(&buf[off]), nlen); off += nlen;
//     if (!get_u32(off, mlen)) return false; off += 4;
//     if (off + mlen > buf.size()) return false;
//     note.assign(reinterpret_cast<const char*>(&buf[off]), mlen); off += mlen;
//     return off == buf.size();
// }

// // ---------------------- I/O 辅助 ----------------------
// static uint64_t now_ms() {
//     using namespace std::chrono;
//     return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
// }

// static bool write_file_header(std::fstream& f) {
//     FileHeader h{};
//     std::memset(h.magic, 0, sizeof(h.magic));
//     std::strncpy(h.magic, MAGIC, sizeof(h.magic)-1);
//     h.version = VERSION;
//     h.header_crc = 0;
//     // 计算 CRC（不含 header_crc 本身）
//     uint8_t tmp[8 + 4];
//     std::memset(tmp, 0, sizeof(tmp));
//     std::memcpy(tmp, h.magic, 8);
//     std::memcpy(tmp + 8, &h.version, 4);
//     h.header_crc = crc32(tmp, sizeof(tmp));

//     f.seekp(0, std::ios::beg);
//     f.write(reinterpret_cast<const char*>(&h), sizeof(h));
//     return f.good();
// }

// static bool read_file_header(std::fstream& f, FileHeader& out) {
//     f.seekg(0, std::ios::beg);
//     if (!f.read(reinterpret_cast<char*>(&out), sizeof(out))) return false;
//     // 校验 magic
//     if (std::strncmp(out.magic, MAGIC, std::strlen(MAGIC)) != 0) return false;
//     // 校验 CRC
//     uint8_t tmp[8 + 4];
//     std::memset(tmp, 0, sizeof(tmp));
//     std::memcpy(tmp, out.magic, 8);
//     std::memcpy(tmp + 8, &out.version, 4);
//     uint32_t c = crc32(tmp, sizeof(tmp));
//     return c == out.header_crc;
// }

// static std::streampos file_end(std::fstream& f) {
//     f.seekg(0, std::ios::end);
//     return f.tellg();
// }

// // ---------------------- 记录级操作 ----------------------
// struct Record {
//     RecordHeader hdr{};
//     std::vector<uint8_t> payload; // 编码后的(name,note)
// };

// static bool write_record(std::fstream& f, const Record& r) {
//     f.seekp(0, std::ios::end);
//     f.write(reinterpret_cast<const char*>(&r.hdr), sizeof(r.hdr));
//     if (!r.payload.empty())
//         f.write(reinterpret_cast<const char*>(r.payload.data()), static_cast<std::streamsize>(r.payload.size()));
//     return f.good();
// }

// static bool read_record_at(std::fstream& f, std::streampos pos, Record& r) {
//     f.seekg(pos, std::ios::beg);
//     if (!f.read(reinterpret_cast<char*>(&r.hdr), sizeof(r.hdr))) return false;
//     r.payload.resize(r.hdr.payload_size);
//     if (r.hdr.payload_size > 0) {
//         if (!f.read(reinterpret_cast<char*>(r.payload.data()), r.hdr.payload_size)) return false;
//     }
//     // 校验 payload CRC
//     if (crc32(r.payload.data(), r.payload.size()) != r.hdr.payload_crc) {
//         std::cerr << "[WARN] Payload CRC mismatch at pos=" << pos << " (id=" << r.hdr.id << ")\n";
//     }
//     return true;
// }

// static bool overwrite_deleted_flag(std::fstream& f, std::streampos pos, uint8_t deleted) {
//     // pos 指向记录头起始，deleted 在 RecordHeader 内第：offset = offsetof(RecordHeader, deleted)
//     const std::streamoff off = static_cast<std::streamoff>(offsetof(RecordHeader, deleted));
//     f.seekp(pos + off, std::ios::beg);
//     f.write(reinterpret_cast<const char*>(&deleted), 1);
//     return f.good();
// }

// // 顺序遍历：返回所有记录位置（便于列表/查找）
// static std::vector<std::streampos> enumerate_record_positions(std::fstream& f) {
//     std::vector<std::streampos> positions;
//     f.seekg(0, std::ios::end);
//     auto end = f.tellg();
//     std::streampos pos = sizeof(FileHeader);
//     while (pos < end) {
//         f.seekg(pos, std::ios::beg);
//         RecordHeader hdr{};
//         if (!f.read(reinterpret_cast<char*>(&hdr), sizeof(hdr))) break;
//         positions.push_back(pos);
//         pos += static_cast<std::streamoff>(sizeof(RecordHeader)) + hdr.payload_size;
//     }
//     return positions;
// }

// // ---------------------- 数据库类 ----------------------
// class BinaryDB {
// public:
//     explicit BinaryDB(const std::string& path) : path_(path) {}

//     bool open() {
//         bool create_new = !fs::exists(path_) || fs::file_size(path_) == 0;
//         file_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
//         if (!file_.is_open()) {
//             // 文件不存在则创建
//             file_.clear();
//             file_.open(path_, std::ios::out | std::ios::binary);
//             file_.close();
//             file_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
//             create_new = true;
//         }
//         if (!file_.is_open()) return false;
//         if (create_new) {
//             return write_file_header(file_);
//         } else {
//             FileHeader h{};
//             return read_file_header(file_, h) && h.version == VERSION;
//         }
//     }

//     bool add(uint32_t id, const std::string& name, const std::string& note) {
//         Record r{};
//         r.hdr.id = id;
//         r.hdr.timestamp = now_ms();
//         r.hdr.deleted = 0;
//         r.payload = encode_payload(name, note);
//         r.hdr.payload_size = static_cast<uint32_t>(r.payload.size());
//         r.hdr.payload_crc = crc32(r.payload.data(), r.payload.size());
//         return write_record(file_, r);
//     }

//     bool get(uint32_t id) {
//         auto positions = enumerate_record_positions(file_);
//         for (auto pos : positions) {
//             Record r{};
//             if (!read_record_at(file_, pos, r)) continue;
//             if (r.hdr.id == id && r.hdr.deleted == 0) {
//                 std::string name, note;
//                 if (!decode_payload(r.payload, name, note)) name = "<decode-error>";
//                 print_record(r, name, note, pos);
//                 return true;
//             }
//         }
//         std::cout << "Record id=" << id << " not found (or deleted).\n";
//         return false;
//     }

//     void list(bool include_deleted=false) {
//         auto positions = enumerate_record_positions(file_);
//         if (positions.empty()) {
//             std::cout << "<no records>\n";
//             return;
//         }
//         for (auto pos : positions) {
//             Record r{};
//             if (!read_record_at(file_, pos, r)) continue;
//             if (!include_deleted && r.hdr.deleted) continue;
//             std::string name, note;
//             if (!decode_payload(r.payload, name, note)) name = "<decode-error>";
//             print_record(r, name, note, pos);
//         }
//     }

//     bool del(uint32_t id) {
//         auto positions = enumerate_record_positions(file_);
//         for (auto pos : positions) {
//             Record r{};
//             if (!read_record_at(file_, pos, r)) continue;
//             if (r.hdr.id == id && r.hdr.deleted == 0) {
//                 if (overwrite_deleted_flag(file_, pos, 1)) {
//                     std::cout << "Deleted id=" << id << " at pos=" << pos << " (logical delete)\n";
//                     return true;
//                 }
//                 return false;
//             }
//         }
//         std::cout << "Record id=" << id << " not found (or already deleted).\n";
//         return false;
//     }

//     void close() { file_.close(); }

// private:
//     void print_record(const Record& r, const std::string& name, const std::string& note, std::streampos pos) {
//         std::time_t tt = static_cast<time_t>(r.hdr.timestamp / 1000);
//         std::tm* ptm = std::localtime(&tt);
//         char tbuf[32]{};
//         if (ptm) std::strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", ptm);
//         std::cout << "pos=" << pos
//                   << ", id=" << r.hdr.id
//                   << ", ts(ms)=" << r.hdr.timestamp
//                   << " (" << tbuf << ")"
//                   << ", deleted=" << int(r.hdr.deleted)
//                   << ", payload_size=" << r.hdr.payload_size
//                   << ", payload_crc=0x" << std::hex << std::setw(8) << std::setfill('0') << r.hdr.payload_crc << std::dec
//                   << "\n    name=" << name << "\n    note=" << note << "\n";
//     }

//     std::string path_;
//     std::fstream file_;
// };

// // ---------------------- CLI ----------------------
// static void print_help() {
//     std::cout << "Commands:\n"
//                  "  add <id> <name> <note...>    : 追加一条记录\n"
//                  "  get <id>                     : 按ID查找记录\n"
//                  "  list [all]                  : 顺序列出记录（加 all 显示删除的）\n"
//                  "  del <id>                    : 按ID逻辑删除\n"
//                  "  help                        : 帮助\n"
//                  "  quit/exit                   : 退出\n";
// }

int main(int argc, char* argv[]) {
    std::string dbpath = "records.bin";
    if (argc >= 2) dbpath = argv[1];

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
