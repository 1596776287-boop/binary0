#include "../include/cage.h"

// ---------------------- 数据库类 ----------------------

    bool BinaryDB::open() {
        bool create_new = !fs::exists(path_) || fs::file_size(path_) == 0;
        file_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
        if (!file_.is_open()) {
            // 文件不存在则创建
            file_.clear();
            file_.open(path_, std::ios::out | std::ios::binary);
            file_.close();
            file_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
            create_new = true;
        }
        if (!file_.is_open()) return false;
        if (create_new) {
            return write_file_header(file_);
        } else {
            FileHeader h{};
            return read_file_header(file_, h) && h.version == VERSION;
        }
    }

    bool BinaryDB::add(uint32_t id, const std::string& name, const std::string& note) {
        Record r{};
        r.hdr.id = id;
        r.hdr.timestamp = now_ms();
        r.hdr.deleted = 0;
        r.payload = encode_payload(name, note);
        r.hdr.payload_size = static_cast<uint32_t>(r.payload.size());
        r.hdr.payload_crc = crc32(r.payload.data(), r.payload.size());
        return write_record(file_, r);
    }

    bool BinaryDB::get(uint32_t id) {
        auto positions = enumerate_record_positions(file_);
        for (auto pos : positions) {
            Record r{};
            if (!read_record_at(file_, pos, r)) continue;
            if (r.hdr.id == id && r.hdr.deleted == 0) {
                std::string name, note;
                if (!decode_payload(r.payload, name, note)) name = "<decode-error>";
                print_record(r, name, note, pos);
                return true;
            }
        }
        std::cout << "Record id=" << id << " not found (or deleted).\n";
        return false;
    }

    void BinaryDB::list(bool include_deleted=false) {
        auto positions = enumerate_record_positions(file_);
        if (positions.empty()) {
            std::cout << "<no records>\n";
            return;
        }
        for (auto pos : positions) {
            Record r{};
            if (!read_record_at(file_, pos, r)) continue;
            if (!include_deleted && r.hdr.deleted) continue;
            std::string name, note;
            if (!decode_payload(r.payload, name, note)) name = "<decode-error>";
            print_record(r, name, note, pos);
        }
    }

    bool BinaryDB::del(uint32_t id) {
        auto positions = enumerate_record_positions(file_);
        for (auto pos : positions) {
            Record r{};
            if (!read_record_at(file_, pos, r)) continue;
            if (r.hdr.id == id && r.hdr.deleted == 0) {
                if (overwrite_deleted_flag(file_, pos, 1)) {
                    std::cout << "Deleted id=" << id << " at pos=" << pos << " (logical delete)\n";
                    return true;
                }
                return false;
            }
        }
        std::cout << "Record id=" << id << " not found (or already deleted).\n";
        return false;
    }

    void BinaryDB::close() { file_.close(); }
    void BinaryDB::print_record(const Record& r, const std::string& name, const std::string& note, std::streampos pos) {
        std::time_t tt = static_cast<time_t>(r.hdr.timestamp / 1000);
        std::tm* ptm = std::localtime(&tt);
        char tbuf[32]{};
        if (ptm) std::strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", ptm);
        std::cout << "pos=" << pos
                  << ", id=" << r.hdr.id
                  << ", ts(ms)=" << r.hdr.timestamp
                  << " (" << tbuf << ")"
                  << ", deleted=" << int(r.hdr.deleted)
                  << ", payload_size=" << r.hdr.payload_size
                  << ", payload_crc=0x" << std::hex << std::setw(8) << std::setfill('0') << r.hdr.payload_crc << std::dec
                  << "\n    name=" << name << "\n    note=" << note << "\n";
    }

