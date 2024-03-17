#include "concrete_net_setup.h"

NetCliLogicContainer *TcpCliConnSetUp::setupConn(NetAddr peer, AbstNetLogic &recv_logic)
{
    // LOG_INFO("TcpCliConnSetUp setupConn: {}",)
    NetCliLogicContainer *udpcli = new NetCliLogicContainer(peer, recv_logic); // ne 何时释放
    NetworkConnCtx *nctx = NetFrameworkImplWithEvent::connectWithTcp(udpcli);                  // nctx 何时释放: 由NetworkLayerWithEvent负责释放
    udpcli->setCtx(nctx);
    return udpcli;
}

void TcpCliConnSetUp::exec(NetworkConnCtx &ctx)
{
    trigger_logic.exec(ctx);
}

NetCliLogicContainer *UdpCliConnSetUp::setupConn(NetAddr peer, AbstNetLogic &recv_logic)
{
    // LOG_INFO("UdpCliConnSetUp setupConn: {}",)
    NetCliLogicContainer *udpcli = new NetCliLogicContainer(peer, recv_logic); // ne 何时释放
    NetworkConnCtx *nctx = NetFrameworkImplWithEvent::connectWithUdp(udpcli);                  // nctx 何时释放: 由NetworkLayerWithEvent负责释放
    udpcli->setCtx(nctx);
    return udpcli;
}

void UdpCliConnSetUp::exec(NetworkConnCtx &ctx)
{
    trigger_logic.exec(ctx);
}
