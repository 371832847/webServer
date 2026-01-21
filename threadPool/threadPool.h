#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<list>
#include<vector>
#include<atomic>

typedef void (*func)(void*);

class task {
public:
	task(func point,void* arg) {
		funcPoint = point;
		data = arg;
	};
	~task() {
		funcPoint = NULL;
	};
	func funcPoint;			//func point
	void* data;				//arg
};

class threadPool {
public:
	threadPool(int taskSize = 1000,int threadNum = 8);	//init
	~threadPool();	//close

	void addTask(func f,void* d);

	void shutdown();

private:
	std::vector<std::thread> workers;	//worker queue
	std::list<task*> list;				//task queue
	int ThreadNum;						//thread num
	std::atomic<int> taskNum;			//task num
	int taskSize;						//task queue size
	std::mutex mtx;						//mutex
	std::condition_variable cv;			//condition_variable
	std::atomic<bool> shutDown;			//thread close flag
};

#endif