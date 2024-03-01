#ifndef __IOUTIL_H_
#define __IOUTIL_H_

#include <sys/fcntl.h>
#include <malloc.h>
#include <unistd.h>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>
#include <filesystem>

#define SIZE_1KB 8196



class IoReadMonitor
{
private:
public:
    IoReadMonitor(/* args */);

    /**
     * A callback that is reading data
     *
     * @note: It is the user's responsibility To free the data
     *
     * @param curpos: is the offset
     * @param data_len: is the readed data len
     * @param : data
     *
     */
    virtual void monitor(uint64_t from_pos, void *data, uint64_t data_len) = 0;
    virtual ~IoReadMonitor() = 0;
};

class IoUtil
{
private:
    std::vector<IoReadMonitor *> readMonitors;

public:
    IoUtil(/* args */);
    ~IoUtil();

    void *readAll(std::string path, uint64_t &ret_len);
    void *readAll(std::string path, uint64_t offset, uint64_t size, uint64_t &ret_len);
    void addReadMonitor(IoReadMonitor *monitor);

    uint64_t writeFile(std::string path, uint64_t offset, void *data, uint64_t size);
};


#endif
