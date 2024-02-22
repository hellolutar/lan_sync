#include "properties_parse.h"

#include <cstring>
#include <fstream>
#include <sstream>

string &trim(string &s)
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
    const int LINE_LENGTH = 1024;
    char buf[LINE_LENGTH];
    while (ifs.getline(buf, LINE_LENGTH))
    {
        for (size_t i = 0; i < strlen(buf); i++)
        {
            char ch = buf[i];
            if (ch == '=')
            {
                string bufstr(buf);
                string key = bufstr.substr(0, i);
                string value = bufstr.substr(i + 1, bufstr.size());
                properties[trim(key)] = trim(value);
                break;
            }
        }
        memset(buf, 0, LINE_LENGTH);
    }
    ifs.close();
}

PropertiesParse::~PropertiesParse()
{
}

string PropertiesParse::query(string key)
{
    return properties[key];
}
