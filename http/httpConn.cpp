#include"httpConn.h"

using namespace std;

const char* httpConn::srcDir;
std::atomic<int> httpConn::userCount;
bool httpConn::isET;

httpConn::httpConn(){
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
}


httpConn::~httpConn(){
    this->Close();
}

void httpConn::init(int fd,const sockaddr_in& addr){
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.retrieve_all();
    readBuff_.retrieve_all();
    isClose_ = false;
    
    // string info;
    // info.append("init client : " + std::to_string(fd_));
    // LOG_INFO(info.c_str());
}

void httpConn::Close(){
    response_.unmap_file();
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        // string info;
        // info.append("close client : " + std::to_string(fd_));
        // LOG_INFO(info.c_str());

        close(fd_);
    }
}

int httpConn::get_fd() const{
    return fd_;
}

sockaddr_in httpConn::get_addr() const{
    return addr_;
}

const char* httpConn::get_ip() const {
    return inet_ntoa(addr_.sin_addr);
}

int httpConn::get_port() const{
    return ntohs(addr_.sin_port);
}

ssize_t httpConn::read(int* saveErrno){
    ssize_t len = -1;
    do{
        len = readBuff_.read_fd(fd_,saveErrno);
        if(len <= 0){
            break;
        }
    }while(isET);

    return len;
}

ssize_t httpConn::write(int* saveErrno){

    ssize_t len = -1;
    ssize_t ret = 0;
    do{
        
        len = writev(fd_,iov_,iovCnt_);
        
        if(len <= 0){
            if(saveErrno != NULL){
                *saveErrno = errno;
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    return len;
                }
            }
        }else{
            //cout << "send len:" << len << endl;
        }

        if(iov_[0].iov_len + iov_[1].iov_len == 0){break;}
        else if(static_cast<size_t>(len) > iov_[0].iov_len){
            iov_[1].iov_base = (uint8_t*)iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len){
                writeBuff_.retrieve_all();
                iov_[0].iov_len = 0;
            }
        }else{
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_.retrieve(len);
        }

    }while(isET && to_write_bytes());
    return len;
}

bool httpConn::process(){
    request_.init();
    if(readBuff_.readable_bytes() <= 0){
        return false;
    }else if(request_.parse(readBuff_)){
        
        if(request_.is_partial()){
            response_.init(srcDir,request_.path(),request_.is_keep_alive(),206);
            response_.range = request_.ranges();
        }else{
            response_.init(srcDir,request_.path(),request_.is_keep_alive(),200);
        }
    }else{
        response_.init(srcDir,request_.path(),false,400);
    }

    response_.make_response(writeBuff_);

    string str(writeBuff_.peek(),writeBuff_.readable_bytes());
    //cout << "response: " << str << endl;
    iov_[0].iov_base = const_cast<char*>(writeBuff_.peek());
    iov_[0].iov_len = writeBuff_.readable_bytes();
    iovCnt_ = 1;

    if(response_.file_len() > 0 && response_.file()){
        if(request_.is_partial()){
            iov_[1].iov_base = response_.file() + response_.base;
            iov_[1].iov_len = response_.offset - response_.base;
        }else{
            iov_[1].iov_base = response_.file();
            iov_[1].iov_len = response_.file_len();
        }
        iovCnt_ = 2;
    }
    return true;
}

int httpConn::to_write_bytes(){
    return iov_[0].iov_len + iov_[1].iov_len;
}

bool httpConn::is_keep_alive() const{
    return request_.is_keep_alive();
}

string httpConn::ulong_to_hex(unsigned long number){

    if(number == 0){
        return string("0");
    }
    stringstream str;
    str << std::hex << number;
    return str.str();
}