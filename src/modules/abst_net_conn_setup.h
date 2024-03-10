#ifndef __ABST_NET_CONN_SETUP_H_
#define __ABST_NET_CONN_SETUP_H_

#include "net/net_logic_container.h"


class AbstNetConnSetup
{
public:
    virtual ~AbstNetConnSetup(){};
    virtual NetCliLogicContainer *setupConn(NetAddr peer, AbstNetLogic &recv_logic) = 0;
};

#endif