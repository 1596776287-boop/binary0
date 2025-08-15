#include "../include/cage.h"



void init_crc32() {
    if (crc32_init_done) return;
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (size_t j = 0; j < 8; ++j) {
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
    crc32_init_done = true;
}
uint32_t crc32(const uint8_t* data, size_t len, uint32_t seed ){
    init_crc32();
    uint32_t c = seed;
    for (size_t i = 0; i < len; ++i) {
        c = crc32_table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFFu;
}