#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>

#define IP_ADDR "14.103.168.41"
#define PORT 8900

int main(){
    static int count = 0;
    for(int i = 0;i < 10;i++){
        count++;
        int fd = socket(AF_INET,SOCK_STREAM,0);
        sockaddr_in addr;
        memset(&addr,0,sizeof(addr));
        addr.sin_addr.s_addr = inet_addr(IP_ADDR);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        connect(fd,(sockaddr*)&addr,sizeof(addr));
        char str[10] = {0};
        recv(fd,str,10,0);
        std::cout << str  << "\t" << count << std::endl;
        close(fd);
    }
    return 0;
}