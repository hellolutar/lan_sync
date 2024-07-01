#ifndef __RANGE_H_
#define __RANGE_H_

#include <cstdint>

class Range2
{
private:
    std::uint64_t start_;
    std::uint64_t end_;

public:
    Range2(std::uint64_t start, std::uint64_t end) : start_(start), end_(end){};
    ~Range2();

    bool operator==(const Range2 &r) const;
    bool operator<(const Range2 &r) const;

    const uint64_t size() const;
};

#endif