#include "crc.h"

// CRC32 Table for polynomial 0xEDB88320
static uint32_t crc32_table[256];

// Function to initialize the CRC32 table
void init_crc32_table() {
    for (int i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        }
        crc32_table[i] = crc;
    }
}

// Function to calculate CRC32
uint32_t calculate_crc32(const void* data, size_t size) {
    const uint8_t* buffer = static_cast<const uint8_t*>(data);
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < size; i++) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ buffer[i]) & 0xFF];
    }

    return crc ^ 0xFFFFFFFF;
}