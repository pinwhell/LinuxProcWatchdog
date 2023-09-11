#pragma once

#include <cstdint>

void init_crc32_table();
uint32_t calculate_crc32(const void* data, size_t size);