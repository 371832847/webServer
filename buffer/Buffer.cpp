#include"Buffer.h"


Buffer::Buffer(int initBufferSize) : buffer(initBufferSize),readPtr(0),writePtr(0){}


size_t Buffer::writeable_bytes() const{
    return buffer.size() - writePtr;
}


size_t Buffer::readable_bytes() const{
    return writePtr - readPtr;
}

size_t Buffer::prependable_bytes() const{
    return readPtr;
}

//readable start ptr
const char* Buffer::peek() const{
    return _begin_ptr() + readPtr;
}

void Buffer::ensure_writeable(size_t len){
    assert(len >= 0);
    if(writeable_bytes() < len){
        _make_space(len);
    }
    assert(writeable_bytes() >= len);
}

void Buffer::has_written(size_t len){
    assert(len >= 0);
    writePtr += len;
}

void Buffer::retrieve(size_t len){
    assert(len >= 0 );
    if(readPtr + len < writePtr){
         readPtr += len;
    }else{
        retrieve_all();
    }
}


void Buffer::retrieve_until(const char* end){
    assert(peek() <= end);
    retrieve(end - peek());
}

void Buffer::retrieve_all(){
    bzero(&buffer[0],buffer.size());
    readPtr = 0;
    writePtr = 0;
}

std::string Buffer::retrieve_all_to_string(){
    std::string str(peek(),readable_bytes());
    retrieve_all();
    return str;
}

const char* Buffer::begin_write_const() const{
    return _begin_ptr() + writePtr;
}

char* Buffer::begin_write(){
    return _begin_ptr() + writePtr;
}

void Buffer::append(const char* str,size_t len){
    assert(str != NULL && len >= 0);
    ensure_writeable(len);
    std::copy(str,str + len,begin_write());
    has_written(len);
}


void Buffer::append(const std::string& str){
    append(str.data(),str.length());
}

void Buffer::append(const void* data,size_t len){
    assert(data);
    append(static_cast<const char*>(data),len);
}

void Buffer::append(const Buffer& buff){
    append(peek(),readable_bytes());
}

ssize_t Buffer::read_fd(int fd,int* saveError){
    char buff[65535];
    struct iovec iov[2];
    size_t writeable = writeable_bytes();
    iov[0].iov_base = begin_write();
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = 65535;

    ssize_t len = readv(fd,iov,2);
    if(len < 0){
        *saveError = errno;
    }else if(len <= writeable){
        has_written(len);
    }else if(static_cast<size_t>(len) > writeable){
        writePtr = buffer.size();
        append(buff,static_cast<size_t>(len) - writeable);
    }
    return len;
}

ssize_t Buffer::write_fd(int fd,int* saveError){
    size_t readable = readable_bytes();
    ssize_t len = write(fd,peek(),readable);
    if(len < 0){
        *saveError = errno;
    }else{
        readPtr += len;
    }
    return len;
}


char* Buffer::_begin_ptr(){
    return &*buffer.begin();
}

void Buffer::_make_space(size_t len){
    if(readPtr + writeable_bytes() < len){
        buffer.resize(writePtr + len - 1);
    }else{
        std::copy(_begin_ptr()+readPtr,_begin_ptr()+writePtr,_begin_ptr());
        writePtr = readable_bytes();
        readPtr = 0;
    }
}

const char* Buffer::_begin_ptr() const{
    return &*buffer.begin();
}