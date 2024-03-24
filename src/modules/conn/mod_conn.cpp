#include "mod_conn.h"

void ModConnAbility::setName(std::string n)
{
    this->name = n;
}
void ModConnAbility::setMediator(AbsModConnMediator *m)
{
    this->med = m;
}

void ModConnAbility::mod_conn_send(std::string to, std::string uri, void *data)
{
    med->mod_tel(name, to, uri, data);
}