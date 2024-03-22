#ifndef __MOD_CONN_H_
#define __MOD_CONN_H_

#include <string>

class AbsModConnMediator
{
public:
    virtual ~AbsModConnMediator(){};
    virtual bool mod_tel(std::string from, std::string to, std::string uri, void *data) = 0;
};

class ModConnAbility
{
protected:
    AbsModConnMediator &med;

public:
    std::string name;

    ModConnAbility(AbsModConnMediator &med, std::string name) : med(med), name(name){};
    virtual ~ModConnAbility(){};
    virtual void mod_conn_recv(std::string from, std::string uri, void *data) = 0;
    virtual void mod_conn_send(std::string to, std::string uri, void *data) = 0;
};

#endif
