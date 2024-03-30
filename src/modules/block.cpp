#include "block.h"
#include <cstring>
#include "utils/str_utils.h"

using namespace std;

bool operator==(const Block &l, const Block &r)
{
    return l.start == r.start && l.end == r.end;
}

bool Block::operator==(const Block &other)
{
    return start == other.start && end == other.end;
}

std::string Block::str()
{
    StringBuilder sb;
    sb.add("[");
    sb.add(to_string(start));
    sb.add(",");
    sb.add(to_string(end));
    sb.add("]");
    return sb.str();
}
