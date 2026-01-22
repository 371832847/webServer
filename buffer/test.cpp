#include<iostream>
#include<fcntl.h>
#include<sys/stat.h>
#include"Buffer.h"
using std::cout;
using std::endl;

int main(){

    Buffer buff;
    int err = 0;
    buff.append("hello\n",6);
    cout << "readable:" << buff.readable_bytes() << "\twriteable:"<<buff.writeable_bytes() << endl;

    int fd = open("../file.txt",O_RDWR);
    if(fd < 0){
        err = errno;
        cout << strerror(err) << endl;
        return -1;
    }
    if(!buff.write_fd(fd,&err)){
        cout << strerror(err) << endl;
        return -1;
    }
    cout << "readable:" << buff.readable_bytes() << "\twriteable:"<<buff.writeable_bytes() << endl;


    buff.append("wwwaaas\n",8);
    cout << "readable:" << buff.readable_bytes() << "\twriteable:"<<buff.writeable_bytes() << endl;
    if(!buff.write_fd(fd,&err)){
        cout << strerror(err) << endl;
        return -1;
    }
    cout << "readable:" << buff.readable_bytes() << "\twriteable:"<<buff.writeable_bytes() << endl;

    close(fd);

    return 0;
}