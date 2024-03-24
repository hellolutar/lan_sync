#include "block.h"

bool operator==(const Block &l, const Block &r)
{
    return l.start == r.start && l.end == r.end;
}

bool Block::operator==(const Block &other)
{
    return start == other.start && end == other.end;
}
