#include"server.h"


sv::server::server(int port,int trigMode,int timeoutMS,bool optLinger,
            int sqlPort,const char* sqlUser,const char* sqlPwd,
            const char* dbName,int connPoolNum,int threadNum) : port_(port)
            ,timeoutMS_(timeoutMS),threads_(new threadPool(threadNum)),sqls_(new sqlConn),
            epoll_(new epoller(10000)),timer_(new HeapTimer){
    srcDir_ = "../src";
    httpConn::userCount = 0;
    httpConn::srcDir = srcDir_;
    sqlConn::instance();
    init_event_mode_(trigMode);
    isClose_ = false;
    if(!init_socket_()){isClose_ = true;}
    Log* l = Log::Instance();
    l->init(1);
    LOG_INFO("============= server init ==============");
}


sv::server::~server(){
    close(listenFd_);
    //free(srcDir_);
    isClose_ = true;

    LOG_INFO("============= server close ==============");
}

void sv::server::init_event_mode_(int trigMode){
    listenEvent_ = EPOLLRDHUP;  //监听半关闭和关闭连接事件
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP; //EPOLLONESHOT 只触发一次后续需要重新注册事件
    switch(trigMode){
        case 0:
            break;
        case 1:
            connEvent_ |= EPOLLET;      //连接用ET
            break;
        case 2:
            listenEvent_ |= EPOLLET;    //监听用ET
            break;
        case 3:
            listenEvent_ |= EPOLLET;    //监听和连接都用ET
            connEvent_ |= EPOLLET;
            break;
        default:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
    }

    httpConn::isET = (connEvent_ & EPOLLET);    //监听描述符ET
}

void sv::server::start(){

    int timeMS = -1;
    while(!isClose_){

        if(timeoutMS_ > 0){
            timeMS = timer_->GetNextTick();
        }
        int eventCnt = epoll_->wait(timeMS);

        for(int i = 0;i < eventCnt; i++){
            
            int fd = epoll_->get_event_fd(i);
            uint32_t events = epoll_->get_events(i);

            if(fd == listenFd_){
                deal_listen_();
            }else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                assert(users_.count(fd) > 0);
                
                std::string info;
                info.append("close fd : " + std::to_string(fd));
                LOG_INFO(info.c_str());
               
                close_conn_(&users_[fd]);

            }else if(events & EPOLLIN){
                assert(users_.count(fd) > 0);

                std::string info;
                info.append("read fd : " + std::to_string(fd));
                LOG_INFO(info.c_str());

                deal_read_(&users_[fd]);
            }else if(events & EPOLLOUT){
                assert(users_.count(fd) > 0);
                std::string info;
                info.append("write fd : " + std::to_string(fd));
                LOG_INFO(info.c_str());
                deal_write_(&users_[fd]);
            }
        }
    }
}

void sv::server::send_error(int fd,const char* info){
    assert(fd > 0);
    int ret = send(fd,info,strlen(info),0);
    if(ret < 0){
        std::string info;
        info.append("send  error fd : " + std::to_string(fd));
        LOG_INFO(info.c_str());
    }
    close(fd);
}

void sv::server::close_conn_(httpConn* client){
    assert(client);
    epoll_->del_fd(client->get_fd());
    client->Close();
}

void sv::server::add_client_(int fd,sockaddr_in addr){
    assert(fd > 0);
    users_[fd].init(fd,addr);
    if(timeoutMS_ > 0){
        timer_->add(fd,timeoutMS_,std::bind(&server::close_conn_,this,&users_[fd]));
    }
    epoll_->add_fd(fd, EPOLLIN | connEvent_);
    set_fd_nonblock_(fd);
}

void sv::server::deal_listen_(){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    do{
        int fd = accept(listenFd_,(struct sockaddr*)&addr,&len);
        if(fd <= 0 ){return;}
        else if(httpConn::userCount >= MAX_FD){
            send_error(fd,"server busy!");
            return;
        }
        
        add_client_(fd,addr);
        
    }while(listenEvent_ & EPOLLET);
}

void sv::server::deal_read_(httpConn* client){
    assert(client);
    extent_time_(client);
    threads_->add_task(std::bind(&sv::server::on_read_,this,client));
}

void sv::server::deal_write_(httpConn* client){
    assert(client);
    extent_time_(client);
    threads_->add_task(std::bind(&sv::server::on_write_,this,client));
}


void sv::server::extent_time_(httpConn* client){
    assert(client);
    if(timeoutMS_ > 0){ timer_->adjust(client->get_fd(),timeoutMS_); }
}

void sv::server::on_read_(httpConn* client){
    assert(client);
    int ret  = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);

    if(ret <= 0 && readErrno != EAGAIN){
        close_conn_(client);
        return;
    }
    on_process(client);
}

void sv::server::on_process(httpConn* client){
    if(client->process()){
        epoll_->mod_fd(client->get_fd(), connEvent_ | EPOLLOUT);
    }else{
        epoll_->mod_fd(client->get_fd(), connEvent_ | EPOLLIN);
    }
}

void sv::server::on_write_(httpConn* client){
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->to_write_bytes() == 0){
        if(client->is_keep_alive()){
            on_process(client);
            return;
        }
    }else if(ret < 0){
        if(writeErrno == EAGAIN){
            epoll_->mod_fd(client->get_fd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    close_conn_(client);
}


bool sv::server::init_socket_(){
    int ret;
    struct sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024){              //判断端口可用性
        std::cout << "port error" << std::endl;
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    struct linger optLinger = { 0 };
    if(openLinger_){                //关闭连接时是否发送内核缓冲区的数据
        optLinger.l_onoff = 1;      //发送开关
        optLinger.l_linger = 1;     //等待时间  时间结束后强制关闭连接
    }

    listenFd_ = socket(AF_INET,SOCK_STREAM,0);
    if(listenFd_ < 0){
        std::cout << "init listen fd error" << std::endl;
        return false;
    }

    ret = setsockopt(listenFd_,SOL_SOCKET,SO_LINGER,&optLinger,sizeof(optLinger));      //优雅关闭

    if(ret < 0){
        close(listenFd_);
        return false;
    }

    int optval = 1;

    ret = setsockopt(listenFd_,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int));   //设置地址复用 方便快速重启程序
    if(ret < 0){
        std::cout << "setsocketopt error" << std::endl;
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_,(struct sockaddr*)&addr,sizeof(addr));
    if(ret < 0){
        std::cout << "bind error" << std::endl;
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_,20);     //listen第二个参数为未处理连接队列的最大值  超过此数量的连接可能会被丢弃
    if(ret < 0){
        std::cout << "listen error" << std::endl;
        close(listenFd_);
        return false;
    }

    ret = epoll_->add_fd(listenFd_, listenEvent_ | EPOLLIN);
    if(ret == 0){
        std::cout << "add listen error" << std::endl;
        close(listenFd_);
        return false;
    }

    set_fd_nonblock_(listenFd_);
    return true;
}

int sv::server::set_fd_nonblock_(int fd){
    
    assert(fd > 0);

    int flags = fcntl(fd,F_GETFL,0);
    if(flags == -1){
        return -1;
    }
    return fcntl(fd,F_SETFL,flags | O_NONBLOCK);     
}