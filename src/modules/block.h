#ifndef __LAN_SYNC_BLOCK_H_
#define __LAN_SYNC_BLOCK_H_

#include <cstdint>

class Block
{
public:
    uint64_t start;
    uint64_t end;
    Block(){};
    Block(uint64_t start, uint64_t end) : start(start), end(end){};
    ~Block(){};

    bool operator==(const Block &other);
};

#endif