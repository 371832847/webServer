#ifndef SERVER_H
#define SERVER_H
#include"../http/httpConn.h"
#include"../sqlConn/sqlConn.h"
#include"../threadPool/threadPool.h"
#include"../epoller/epoller.h"
#include"../heapTimer/heaptimer.h"
#include"../log/log.h"

#include<unordered_map>


namespace sv{


#define MAX_FD 1000000

class server{
public:
    server(int port,int trigMode,int timeoutMs,bool optLinger,
            int sqlPort,const char* sqlUser,const char* sqlPwd,
            const char* dbName,int connPoolNum = MAX_FD,int threadNum = 16);
    ~server();
    void start();
    
private:

    bool init_socket_();

    void init_event_mode_(int trigMode);
    
    void add_client_(int fd,sockaddr_in addr);

    void deal_listen_();

    void deal_write_(httpConn* client);

    void deal_read_(httpConn* client);

    void send_error(int fd,const char* info);

    void extent_time_(httpConn* client);

    void close_conn_(httpConn* client);

    void on_read_(httpConn* client);
    void on_write_(httpConn* client);
    void on_process(httpConn* client);

    static int set_fd_nonblock_(int fd);

private:

    int port_;
    bool openLinger_;
    int timeoutMS_;
    bool isClose_;
    int listenFd_;       
    char* srcDir_;          //src

    uint32_t listenEvent_;      //监听套接字配置
    uint32_t connEvent_;        //连接套接字配置


    std::unique_ptr<HeapTimer> timer_;      //定时器       
    std::unique_ptr<sqlConn> sqls_;         //数据库连接池
    std::unique_ptr<threadPool> threads_;   //线程池
    std::unique_ptr<epoller> epoll_;        //epoll
    std::unordered_map<int,httpConn> users_;// fd,httpConn
};

}

#endif //SERVER_H