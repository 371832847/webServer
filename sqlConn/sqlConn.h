#ifndef SQLCONN_H
#define SQLCONN_H

#include<mysql/mysql.h>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<vector>

class sqlConn {
public:
	sqlConn(unsigned int size = 8);
	~sqlConn();
	
	MYSQL* getConn();					//获取连接

	bool returnConn(MYSQL* conn);		//归还连接

	unsigned int getAvaliable();		//获取可用的连接数

private:
	std::vector<MYSQL*> pool;			//连接队列
	std::vector<bool> flag;				//记录队列使用情况的结构体
	unsigned int avaliable;				//可用连接数量
	unsigned int connSize;				//数据库连接总数
	std::mutex mtx;						//互斥锁		
};

#endif 

