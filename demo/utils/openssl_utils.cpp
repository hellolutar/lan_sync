#include "openssl_utils.h"

OpensslUtil::OpensslUtil(/* args */)
{
    lib_ctx = OSSL_LIB_CTX_new();
    assert(lib_ctx);
}

OpensslUtil::~OpensslUtil()
{
    if (lib_ctx)
        OSSL_LIB_CTX_free(lib_ctx);
}

// https://blog.csdn.net/QCZL_CC/article/details/119957918
string OpensslUtil::mdEncodeWithSHA3_512(string f)
{
    if (f.size() == 0)
    {
        return "";
    }

    const char *option_properties = nullptr;
    EVP_MD *msg_digest = EVP_MD_fetch(lib_ctx, "SHA3-512", option_properties);

    assert(msg_digest);

    unsigned int digest_len = EVP_MD_get_size(msg_digest);
    assert(digest_len > 0);

    unsigned char *digest_value = (unsigned char *)OPENSSL_malloc(digest_len);

    assert(digest_value);

    EVP_MD_CTX *digest_ctx = EVP_MD_CTX_new();

    assert(digest_ctx);

    assert(EVP_DigestInit(digest_ctx, msg_digest) == 1);

    ifstream ifs(f);
    char buf[4096] = {0};
    while (ifs >> buf)
    {
        assert(EVP_DigestUpdate(digest_ctx, buf, strlen(buf)) == 1);
        memset(buf, 0, 4096);
    }
    assert(EVP_DigestFinal(digest_ctx, digest_value, &digest_len));

    stringstream iss;
    for (int j = 0; j < digest_len; j++)
    {
        char tmpBuf[3] = {0};
        sprintf(tmpBuf, "%02x", digest_value[j]);
        iss << tmpBuf;
    }
    string ret;
    iss >> ret;
    EVP_MD_CTX_free(digest_ctx);
    OPENSSL_free(digest_value);
    EVP_MD_free(msg_digest);

    return ret;
}