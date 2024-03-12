#ifndef __OPENSSL_UTILS_H_
#define __OPENSSL_UTILS_H_


#include <cstdio>
#include <cstring>
#include <cassert>

#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>

#include <openssl/err.h>
#include <openssl/evp.h>

#include "utils/io_utils.h"

using namespace std;

class OpensslUtil
{
private:
    static OSSL_LIB_CTX *lib_ctx;
    OpensslUtil(/* args */){};
    ~OpensslUtil(){};

public:

    static std::string mdEncodeWithSHA3_512(std::string f);
};

#endif