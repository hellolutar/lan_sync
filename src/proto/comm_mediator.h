#ifndef __COMM_MEDIATOR_H_
#define __COMM_MEDIATOR_H_

#include <string>

class CommDto
{
public:
    virtual ~CommDto() = 0;
};

class CommColleage;

class CommMediator
{
private:
public:
    virtual ~CommMediator();
    virtual void tell(CommColleage *from, CommColleage *to, CommDto *dto) = 0;
};

class CommColleage
{
protected:
    CommMediator *med;
    CommColleage(CommMediator *med):med(med){};

public:
    virtual ~CommColleage();

    virtual void comm_recv(CommColleage *from, CommDto *dto) = 0;

    virtual void comm_send(CommColleage *to, CommDto *dto) = 0;
};

#endif