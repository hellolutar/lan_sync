#include "net_cli_req_table_index.h"

using namespace std;
NetCliConnReqTableIndex::NetCliConnReqTableIndex(/* args */)
{
}

NetCliConnReqTableIndex::~NetCliConnReqTableIndex()
{
}

NetCliLogicContainer *NetCliConnReqTableIndex::setupConn(NetAddr peer, Logic &logic)
{
    NetCliLogicContainer *tcpcli = new NetCliLogicContainer(peer, logic);                     // ne 何时释放
    NetworkConnCtx *nctx = NetFrameworkImplWithEvent::connectWithTcp(tcpcli); // nctx 何时释放: 由NetworkLayerWithEvent负责释放
    tcpcli->setCtx(nctx);
    return tcpcli;
}

void NetCliConnReqTableIndex::exec(NetworkConnCtx &ctx)
{
    // TODO
}
