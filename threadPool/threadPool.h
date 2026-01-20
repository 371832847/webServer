#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<list>
#include<vector>
#include<atomic>

typedef void* (*func)(void*);

class task {
public:
	task(func p,void* d,int t) {
		funcPoint = p;
		data = d;
		type = t;
	};
	~task() {
		funcPoint = NULL;
	};
	func funcPoint;			//回调函数指针
	void* data;				//函数参数
	int type;				//函数参数的类型
};

class threadPool {
public:
	threadPool(int taskSize = 1000,int threadNum = 8);	//构造函数
	~threadPool();	//析构函数

	void addTask(func f,void* d);

	void shutdown();

private:
	std::vector<std::thread> workers;	//工作队列
	std::list<task*> list;				//任务队列
	int ThreadNum;						//线程数量
	std::atomic<int> taskNum;			//当前任务数量
	int taskSize;						//任务队列大小
	std::mutex mtx;						//互斥锁
	std::condition_variable cv;			//条件变量
	std::atomic<bool> shutDown;			//线程池销毁标志位
};

#endif