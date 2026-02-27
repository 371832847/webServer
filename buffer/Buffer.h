#ifndef BUFFER_H
#define BUFFER_H

#include<iostream>
#include<vector>
#include<atomic>
#include<assert.h>  
#include<unistd.h>
#include<error.h>   //errno
#include<string.h>  //bzero
#include<sys/uio.h> //readv
/*
*   pre     used        unused
*   ----------------------------------
*   |      |  / / / /  |             |
*   |      |  / / / /  |             |
*   ----------------------------------
*   0   readptr     writeptr        size
*/


class Buffer{
public:
    Buffer(int initBufferSize = 10);   
    ~Buffer() = default;

    size_t writeable_bytes() const;
    size_t readable_bytes() const;
    size_t prependable_bytes() const;

    const char* peek() const;
    void ensure_writeable(size_t len);
    void has_written(size_t len);

    void retrieve(size_t len);
    void retrieve_until(const char* end);
    void retrieve_all();
    std::string retrieve_all_to_string();

    const char* begin_write_const() const;
    char* begin_write();

    void append(const char* str,size_t len);
    void append(const std::string& str);
    void append(const void* data,size_t len);
    void append(const Buffer& buff);

    ssize_t read_fd(int fd,int* error);
    ssize_t write_fd(int fd,int* error);
    size_t size();

private:

    char* _begin_ptr();
    const char* _begin_ptr() const;
    void _make_space(size_t len);

    std::vector<char> buffer;
    std::atomic<std::size_t> readPtr;
    std::atomic<std::size_t> writePtr;
};

#endif //BUFFER_H