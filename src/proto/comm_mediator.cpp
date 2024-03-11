#include "comm_mediator.h"

CommDto::~CommDto()
{
}

CommMediator::~CommMediator(){

}


CommColleage::~CommColleage()
{
    delete med;
}