#include "mod_conn.h"

void ModConnAbility::mod_conn_send(std::string to, std::string uri, void *data)
{
    med->mod_tel(name, to, uri, data);
}