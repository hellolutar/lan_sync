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
bool StringUtils::eq(std::string l, std::string r)
{
    if (l.size() != r.size())
        return false;
    for (size_t i = 0; i < l.size(); i++)
    {
        if (l[i] != r[i])
            return false;
    }
    return true;
}
bool StringUtils::eqIgnoreCase(std::string l, std::string r)
{
    throw "StringUtils::contains() : unsupport";
}
bool StringUtils::contains(std::string l, std::string sub)
{
    throw "StringUtils::contains() : unsupport";
}