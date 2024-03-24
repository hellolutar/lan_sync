#include "abs_buf.h"

AbsBuf::~AbsBuf()
{
    if (d != nullptr)
        delete[] d;
}