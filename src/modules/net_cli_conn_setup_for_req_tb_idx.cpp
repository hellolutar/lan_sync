#include "net_cli_conn_setup_for_req_tb_idx.h"

using namespace std;
NetCliConnSetupForReqTbIdx::NetCliConnSetupForReqTbIdx(/* args */)
{
}

NetCliConnSetupForReqTbIdx::~NetCliConnSetupForReqTbIdx()
{
}

NetCliLogicContainer *NetCliConnSetupForReqTbIdx::setupConn(NetAddr peer, AbstNetLogic &logic)
{
    NetCliLogicContainer *tcpcli = new NetCliLogicContainer(peer, logic);                     // ne 何时释放
    NetworkConnCtx *nctx = NetFrameworkImplWithEvent::connectWithTcp(tcpcli); // nctx 何时释放: 由NetworkLayerWithEvent负责释放
    tcpcli->setCtx(nctx);
    return tcpcli;
}

void NetCliConnSetupForReqTbIdx::exec(NetworkConnCtx &ctx)
{
    printf("NetCliConnSetupForReqTbIdx::exec() todo\n");
}
