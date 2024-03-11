#include "properties_parse.h"

#include <cstring>
#include <fstream>
#include <sstream>

string trim(string s)
{
    if (s.empty())
    {
        return s;
    }

    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

PropertiesParse::PropertiesParse(string filepath)
{
    ifstream ifs(filepath);
    const int LINE_LENGTH = 10240;
    char buf[LINE_LENGTH];
    while (ifs.getline(buf, LINE_LENGTH))
    {
        string line(buf);
        size_t eq_pos = line.find("=");
        string key = line.substr(0, eq_pos);
        string value = line.substr(eq_pos + 1, line.size() - (eq_pos + 1));
        properties[trim(key)] = trim(value);
        memset(buf, 0, LINE_LENGTH);
    }
    ifs.close();
    this->filepath = filepath;
}

PropertiesParse::~PropertiesParse()
{
}

string PropertiesParse::query(string key)
{
    return properties[key];
}
