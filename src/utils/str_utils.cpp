#include "str_utils.h"
#include <sstream>

using namespace std;

StringBuilder StringBuilder::builder()
{
    StringBuilder b;
    return b;
}

StringBuilder &StringBuilder::add(std::string a)
{
    buf.push_back(a);
    return *this;
}
std::string StringBuilder::str()
{
    if (buf.size() == 0)
        return "";

    stringstream ss;
    for (size_t i = 0; i < buf.size(); i++)
    {
        ss << buf[i];
    }

    string ret;
    ss >> ret;
    return ret;
}