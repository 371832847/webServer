#ifndef SQLCONN_H
#define SQLCONN_H

#include<mysql/mysql.h>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<vector>
#include<assert.h>

#define IP_ADDR "14.103.168.41"
#define PORT 8900
#define NAME "root"
#define PWD "371832"
#define DATABASE "webchat"



class sqlConn {
public:
	sqlConn(unsigned int size = 8);
	~sqlConn();
	
	static sqlConn* instance();

	MYSQL* get_conn();					//get connection

	bool return_conn(MYSQL* conn);		//return connection

	unsigned int get_avaliable();		//get avaliable connection num

private:
	std::vector<MYSQL*> pool;			//connection pool
	std::vector<bool> flag;				//every connection status
	unsigned int avaliable;				//avaliable num
	unsigned int connSize;				//connection pool size
	std::mutex mtx;						//mutex		
};

#endif 

