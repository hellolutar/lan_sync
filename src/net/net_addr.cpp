#include "net_addr.h"
#include <cstring>
#include <sstream>

using namespace std;

NetAddr::NetAddr()
{
    memset(&le_addr, 0, sizeof(sockaddr_in));
}

NetAddr::NetAddr(string ipport)
{
    memset(&le_addr, 0, sizeof(sockaddr_in));
    size_t pos = ipport.find(':');
    if (pos == 0)
    {
        le_addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        string ip = ipport.substr(0, pos);
        inet_aton(ip.data(), &(le_addr.sin_addr));
        le_addr.sin_addr.s_addr = ntohl(le_addr.sin_addr.s_addr);
    }
    if (ipport.find(":")!=string::npos)
    {
        string port = ipport.substr(pos + 1, ipport.size());
         le_addr.sin_port = atoi(port.data());
    }
    le_addr.sin_family = AF_INET;
}

// NetAddr::NetAddr(NetAddr &addr)
// {
//     this->le_addr = addr.le_addr;
// }

NetAddr::~NetAddr()
{
}

bool operator<(const NetAddr &l, const NetAddr &r)
{
    return l.le_addr.sin_addr.s_addr < r.le_addr.sin_addr.s_addr;
}

bool NetAddr::operator==(NetAddr &other)
{
    if (this->le_addr.sin_addr.s_addr != other.le_addr.sin_addr.s_addr)
        return false;

    if (this->le_addr.sin_family != other.le_addr.sin_family)
        return false;

    if (this->le_addr.sin_port != other.le_addr.sin_port)
        return false;

    return true;
}
sockaddr_in NetAddr::getBeAddr()
{
    sockaddr_in be_addr;
    memcpy(&be_addr, &le_addr, sizeof(sockaddr_in));
    be_addr.sin_addr.s_addr = htonl(be_addr.sin_addr.s_addr);
    be_addr.sin_port = htons(be_addr.sin_port);
    return be_addr;
}

sockaddr_in NetAddr::getAddr()
{
    return le_addr;
}

string NetAddr::str()
{
    in_addr tmp = le_addr.sin_addr;
    tmp.s_addr = htonl(tmp.s_addr);

    stringstream ss;
    ss << inet_ntoa(tmp) << ":" << to_string(le_addr.sin_port);
    return ss.str();
}

NetAddr NetAddr::fromBe(sockaddr_in be)
{
    uint16_t port = ntohs(be.sin_port);
    string ip = inet_ntoa(be.sin_addr);
    ip += ":";
    ip += to_string(port);
    return NetAddr(ip);
}

void NetAddr::setPort(int le_port)
{
    le_addr.sin_port = le_port;
}

uint16_t NetAddr::getPort()
{
    return le_addr.sin_port;
}
void NetAddr::setIp(string ip)
{
    inet_aton(ip.data(), &(le_addr.sin_addr));
    le_addr.sin_addr.s_addr = ntohl(le_addr.sin_addr.s_addr);
}