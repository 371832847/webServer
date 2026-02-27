#include<iostream>
#include<fcntl.h>
#include<sys/stat.h>
#include"Buffer.h"
using std::cout;
using std::endl;

int main(){

    Buffer buff;
    int err = 0;
    int fd = open("../file.txt",O_RDWR);
    if(fd < 0){
        err = errno;
        cout << strerror(err) << endl;
        return -1;
    }

    if(!buff.read_fd(fd,&err)){
        cout << strerror(err) << endl;
        return -1;
    }
    cout << "readable:" << buff.readable_bytes() << "\twriteable:"
    << buff.writeable_bytes() << "\tsize:" << buff.size()<< endl;


    int to_fd = open("file",O_RDWR);
    if(to_fd < 0){
        err = errno;
        cout << strerror(err) << endl;
        return -1;
    }

    if(!buff.write_fd(to_fd,&err)){
        cout << strerror(err) <<endl;
        return -1;
    }

    cout << "readable:" << buff.readable_bytes() << "\twriteable:"
    << buff.writeable_bytes() << "\tsize:" << buff.size()<< endl;
    cout << buff.retrieve_all_to_string() << endl;

    buff.append("hello buffer\t",13);

    cout << "readable:" << buff.readable_bytes() << "\twriteable:"
    << buff.writeable_bytes() << "\tsize:" << buff.size()<< endl;
    cout << buff.retrieve_all_to_string() << endl;

    close(fd);
    close(to_fd);

    return 0;
}