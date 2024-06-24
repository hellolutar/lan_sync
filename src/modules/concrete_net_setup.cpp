#include "concrete_net_setup.h"

NetCliLogicContainer *TcpCliConnSetUp::setupConn(NetAddr peer, AbstNetLogic &recv_logic)
{
    // LOG_INFO("TcpCliConnSetUp setupConn: {}",)
    NetCliLogicContainer *tcpcli = new NetCliLogicContainer(peer, recv_logic); // ne 何时释放
    std::shared_ptr<NetworkConnCtx> nctx = NetFrameworkImplWithEvent::connectWithTcp(tcpcli);
    if (nctx==nullptr)
        return nullptr;
    tcpcli->setCtx(nctx);
    return tcpcli;
}

void TcpCliConnSetUp::exec(std::shared_ptr<NetworkConnCtx> &ctx)
{
    trigger_logic.exec(ctx);
}

NetCliLogicContainer *UdpCliConnSetUp::setupConn(NetAddr peer, AbstNetLogic &recv_logic)
{
    // LOG_INFO("UdpCliConnSetUp setupConn: {}",)
    NetCliLogicContainer *udpcli = new NetCliLogicContainer(peer, recv_logic); // ne 何时释放
    std::shared_ptr<NetworkConnCtx> nctx = NetFrameworkImplWithEvent::connectWithUdp(udpcli);                  // nctx 何时释放: 由NetworkLayerWithEvent负责释放
    udpcli->setCtx(nctx);
    return udpcli;
}

void UdpCliConnSetUp::exec(std::shared_ptr<NetworkConnCtx> &ctx)
{
    trigger_logic.exec(ctx);
}
