#include "sync_server.h"
#include "sync_cli.h"

struct event_base *base = event_base_new();

SyncServer *sync_server = new SyncServer(base);
SyncCli *sync_cli = new SyncCli(base);

#include <cstdio>

int main(int argc, char const *argv[])
{
    configlog();
    sync_server->start();
    sync_cli->start();

    event_base_free(base);

    free(sync_cli);
    free(sync_server);

    return 0;
}
