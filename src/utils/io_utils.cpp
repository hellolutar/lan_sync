#include "io_utils.h"

using namespace std;

IoUtil::IoUtil(/* args */)
{
}

IoUtil::~IoUtil()
{
    for (size_t i = 0; i < readMonitors.size(); i++)
    {
        auto rdm = readMonitors[i];
        delete rdm;
    }
    // printf("[DEBUG] IoUtil RELEASE\n");
}

void IoUtil::addReadMonitor(IoReadMonitor *monitor)
{
    readMonitors.push_back(monitor);
}

void *IoUtil::readAll(string path, uint64_t &ret_len)
{
    filesystem::path p(path);
    size_t size = filesystem::file_size(p);

    return readSize(path, 0, size, ret_len);
}

void *IoUtil::readSize(string path, uint64_t offset, uint64_t size, uint64_t &ret_len)
{
    ret_len = 0;

    int fd = open(path.data(), O_RDONLY);
    if (fd < 0)
        return nullptr;

    off_t curpos = lseek(fd, offset, SEEK_SET);
    if (curpos != offset)
    {
        return nullptr;
    }

    uint64_t end = offset + size;

    uint8_t *data = (uint8_t *)malloc(size); // free by outter
    uint8_t *data_pos = data;
    memset(data, 0, size);

    while (curpos < end)
    {
        uint64_t start_at = curpos;
        uint64_t remained = end - curpos;

        // 读取文件内容
        uint64_t expected_read = min(once_read_max_num, remained);
        uint64_t actual_read = read(fd, data_pos, expected_read);
        for (size_t i = 0; i < readMonitors.size(); i++)
        {
            IoReadMonitor *rdm = readMonitors[i];
            rdm->monitor(curpos, data_pos, actual_read);
        }
        curpos += actual_read;
        ret_len += actual_read;
        data_pos += actual_read;
    }
    close(fd);

    return data;
}

uint64_t IoUtil::writeFile(string path, uint64_t offset, void *data, uint64_t size)
{
    uint64_t ret_len = 0;
    int fd = open(path.data(), O_RDWR | O_CREAT, 0644);
    if (fd < 0)
        return ret_len;

    off_t curpos = lseek(fd, offset, SEEK_SET);

    uint64_t end = offset + size;
    uint64_t once_write_max_num = SIZE_1KB * 1024;

    uint64_t expected_write = min(once_write_max_num, size);
    uint64_t actual_write = write(fd, data, expected_write);
    curpos += actual_write;
    ret_len += actual_write;

    while (curpos < end)
    {
        uint64_t remained = end - curpos;

        expected_write = min(once_write_max_num, remained);
        actual_write = write(fd, data, expected_write);
        curpos += actual_write;
        ret_len += actual_write;
    }

    close(fd);

    return ret_len;
}

IoReadMonitor::~IoReadMonitor()
{
}

IoReadMonitor::IoReadMonitor()
{
}