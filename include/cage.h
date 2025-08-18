#ifndef CAGE_H
#define CAGE_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;
extern uint32_t crc32_table[256];
extern const char* MAGIC ;
 const uint32_t VERSION = 1;
extern bool crc32_init_done;

void init_crc32();
uint32_t crc32(const uint8_t* data, size_t len, uint32_t seed = 0xFFFFFFFFu);
#pragma pack(push, 1)
struct FileHeader {
    char     magic[8];       // "BINDATA" + NUL（或类似标识）
    uint32_t version;        // 版本号
    uint32_t header_crc;     // 头部校验（覆盖 magic 和 version，不含本字段）
};

struct RecordHeader {
    uint32_t id;             // 记录ID
    uint64_t timestamp;      // 记录时间（epoch毫秒）
    uint8_t  deleted;        // 0=有效, 1=逻辑删除
    uint32_t payload_size;   // 载荷字节数
    uint32_t payload_crc;    // 只对载荷做CRC（便于修改deleted而不破坏校验）
};
#pragma pack(pop)
std::vector<uint8_t> encode_payload(const std::string& name, const std::string& note);
bool decode_payload(const std::vector<uint8_t>& buf, std::string& name, std::string& note);
uint64_t now_ms();
bool write_file_header(std::fstream& f);
bool read_file_header(std::fstream& f, FileHeader& out);
std::streampos file_end(std::fstream& f);
struct Record {
    RecordHeader hdr{};
    std::vector<uint8_t> payload; // 编码后的(name,note)
};
bool write_record(std::fstream& f, const Record& r);
bool read_record_at(std::fstream& f, std::streampos pos, Record& out);
bool overwrite_deleted_flag(std::fstream& f, std::streampos pos, uint8_t deleted);
std::vector<std::streampos> enumerate_record_positions(std::fstream& f);
class BinaryDB{
    public:
    explicit BinaryDB(const std::string& path): path_(path) {};
    bool open();
    bool add(uint32_t id, const std::string& name, const std::string& note);
    bool get(uint32_t id);
    void list(bool include_deleted);
    bool del(uint32_t id);
    void close();
private:
    void print_record(const Record& r, const std::string& name, const std::string& note, std::streampos pos);
    std::string path_;
    std::fstream file_;
};
void print_help();
void encrypt(std::string in, std::string out, std::string key);


#endif // CAGE_H