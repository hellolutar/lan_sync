#ifndef __SYNC_COMMON_H_
#define __SYNC_COMMON_H_

#include <cstdint>

const uint64_t SIZE_1KByte = 1024;
const uint64_t SIZE_50_KByte = 50 * SIZE_1KByte;
const uint64_t SIZE_1MByte = 1024 * SIZE_1KByte;

const uint64_t BLOCK_SIZE = SIZE_1MByte;

const uint8_t DOWNLOAD_LIMIT = 255;

const uint8_t MAX_RETRY = 5;

#endif