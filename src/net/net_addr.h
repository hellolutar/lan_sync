#ifndef __NET_ADDR_H_
#define __NET_ADDR_H_

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class NetAddr
{
private:
    sockaddr_in le_addr;

public:
    NetAddr();
    // NetAddr(NetAddr &addr);
    NetAddr(std::string ipport);
    NetAddr(sockaddr_in le_addr) : le_addr(le_addr){};
    ~NetAddr();
    sockaddr_in getBeAddr();
    sockaddr_in getAddr();
    friend bool operator<(const NetAddr &l, const NetAddr &r);
    bool operator==(NetAddr &p);

    std::string str();

    static NetAddr fromBe(sockaddr_in be);
};

#endif