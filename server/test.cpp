#include"server.h"



/*
*   视频传输问题 已解决
    登录问题    暂未解决
    注册问题    暂未解决
    高并发时服务器死机  暂未解决
*/


/*
server(int port,int trigMode,int timeoutMs,bool optLinger,
            int sqlPort,const char* sqlUser,const char* sqlPwd,
            const char* dbName,int connPoolNum = MAX_FD,int threadNum = 8);
*/

int main(){

    sv::server s(8900,3,120,true,3306,"root","371832","account");
    s.start();

    return 0;
}