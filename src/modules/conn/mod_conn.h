#ifndef __MOD_CONN_H_
#define __MOD_CONN_H_

#include <string>

class ModConnAbility;

class AbsModConnMediator
{
public:
    virtual ~AbsModConnMediator(){};
    virtual bool mod_tel(std::string from, std::string to, std::string uri, void *data) = 0;
    virtual bool add(ModConnAbility *) = 0;
    virtual void del(std::string name) = 0;
};

class ModConnAbility
{
protected:
    AbsModConnMediator *med;

public:
    std::string name;
    ModConnAbility(){};
    ModConnAbility(AbsModConnMediator *med, std::string name) : med(med), name(name){};
    virtual ~ModConnAbility(){};
    virtual void setName(std::string n);
    virtual void setMediator(AbsModConnMediator *m);
    virtual void mod_conn_recv(std::string from, std::string uri, void *data) = 0;
    virtual void mod_conn_send(std::string to, std::string uri, void *data);
};

#endif
