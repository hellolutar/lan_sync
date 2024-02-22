#ifndef __PROPERTIES_PARSE_H_
#define __PROPERTIES_PARSE_H_

#include <string>
#include <map>

using namespace std;

class PropertiesParse
{
private:
    string filepath;
    map<string, string> properties;

public:
    PropertiesParse(string filepath);
    ~PropertiesParse();

    string query(string key);
};

#endif