#ifndef HTTPCONN_H
#define HTTPCONN_H

#include<arpa/inet.h>
#include<unistd.h>


#include"httpRequest.h"
#include"httpResponse.h"
#include"../log/log.h"


#define CHUNK_SIZE  64*1024

class httpConn{
public:
    httpConn();
    ~httpConn();

    void init(int sockFd,const sockaddr_in& addr);

    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);

    void Close();

    int get_fd() const;

    int get_port() const;

    const char* get_ip() const;

    sockaddr_in get_addr() const;

    bool process();

    int to_write_bytes();

    bool is_keep_alive() const;

    static bool isET;
    static size_t offset;
    static const char* srcDir;
    static std::atomic<int> userCount;

private:

    std::string ulong_to_hex(unsigned long number);
    int fd_;
    struct sockaddr_in addr_;
    bool isClose_;
    int iovCnt_;
    struct iovec iov_[2];

    Buffer readBuff_;
    Buffer writeBuff_;

    httpRequest request_;
    httpResponse response_;
};


#endif //HTTPCONN_H
