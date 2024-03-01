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

using namespace std;

class OpensslUtil
{
private:
    OSSL_LIB_CTX *lib_ctx;

public:
    OpensslUtil(/* args */);
    ~OpensslUtil();

    string mdEncodeWithSHA3_512(string f);
};

#endif