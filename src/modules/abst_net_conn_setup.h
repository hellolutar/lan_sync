#ifndef __ABST_NET_CONN_SETUP_H_
#define __ABST_NET_CONN_SETUP_H_

#include "modules/net_cli_logic_container.h"


class AbstNetConnSetup
{
public:
    virtual ~AbstNetConnSetup(){};
    virtual NetCliLogicContainer *setupConn(NetAddr peer, AbstNetLogic &recv_logic) = 0;
};

#endif