#include "modules/task2/range.h"

Range2::~Range2()
{
}

bool Range2::operator==(const Range2 &r) const
{
    return r.start_ == start_ && r.end_ == end_;
}

bool Range2::operator<(const Range2 &r) const
{
    if (r.start_ != start_)
        return false;

    return end_ < r.end_;
}

const uint64_t Range2::size() const
{
    return end_ - start_;
}
