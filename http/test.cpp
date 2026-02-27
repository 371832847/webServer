#include"httpResponse.h"
#include"../buffer/Buffer.h"
#include"httpRequest.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include"httpConn.h"
#include"../epoller/epoller.h"

int main(){

    epoller e;
    httpConn::srcDir = "../../src";
    httpConn h;
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd < 0){
        std::cout << "socket init error" << std::endl;
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8900);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(fd,(sockaddr*)&addr,sizeof(addr)) != 0){
        std::cout << "bind error" << std::endl;
        return -1;
    }

    if(listen(fd,5) != 0){
        std::cout << "listen error" << std::endl;
        return -1;
    }

    int optval = 1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int));
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);

    if(!e.add_fd(fd,EPOLLIN | EPOLLET)){
        std::cout << "add epoll error" << std::endl;
        return -1;
    }
    int count = 0;
    while(1){

        int num = e.wait();
        for(int i = 0; i < num; i++){

            int server = e.get_event_fd(i);
            int event = e.get_events(i);
            if(event & EPOLLIN && server == fd){
                count++;
                struct sockaddr_in add;
                socklen_t len = sizeof(add);
                int client = accept(server,(sockaddr*)&add,&len);
                if(client < 0){
                    std::cout << "client error" << std::endl;
                    return -1;
                }
                h.init(client,add);
                std::cout << "fd:" << h.get_fd() << "\tip:" << h.get_ip() << "\tport:" <<h.get_port() << std::endl;
                std::cout << "read bytes:" << h.read(0) << std::endl;
                h.process();
                std::cout << "write bytes:" << h.write(0) << std::endl;
            }
        }
    }
    return 0;
}