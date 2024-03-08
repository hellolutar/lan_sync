#ifndef __SYNC_H_
#define __SYNC_H_

#include "comm/comm_mediator.h"

class Sync:public CommMediator
{
private:
public:
    Sync(/* args */);
    ~Sync();
};

Sync::Sync(/* args */)
{
}

Sync::~Sync()
{
}

#endif