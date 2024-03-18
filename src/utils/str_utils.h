#ifndef __STR_UTILS_H_
#define __STR_UTILS_H_

#include <string>
#include <vector>

class StringBuilder
{
private:
    std::vector<std::string> buf;

public:
    StringBuilder(){};
    ~StringBuilder(){};
    StringBuilder &add(std::string);
    static StringBuilder builder();
    std::string str();
};
#endif