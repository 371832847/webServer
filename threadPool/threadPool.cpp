#include "threadPool.h"

threadPool::threadPool(int taskSize,int threadNum){
	this->taskSize = taskSize;
	this->ThreadNum = threadNum;
	taskNum = 0;
	shutDown = false;
	for (int i = 0; i < threadNum; ++i) {
		
		workers.push_back(std::thread([this]() {

			while (true) {

				//执行任务
				{
					//取任务
					std::unique_lock<std::mutex> m(mtx);
					while (taskNum == 0 && !shutDown) {
						cv.wait(m);
					}
					
					//判断是否退出
					if (shutDown) {
						break;
					}

					if (taskNum == 0)
						continue;

					task* temp = list.front();
					list.pop_front();
					taskNum--;
					m.unlock();
					if (taskNum + 1 == this->taskSize || taskNum == 0) {		//如果取任务前队列满 通知进程添加任务
						cv.notify_all();
					}
					temp->funcPoint(temp->data);
					delete temp;
				}
			}
			}));
	}
}

threadPool::~threadPool() {

	shutDown = true;
	cv.notify_all();
	for (auto it = workers.begin(); it != workers.end(); ++it) {
		it->join();
	}

	for (int i = 0; i < taskNum; i++) {
		task* t = list.front();
		list.pop_front();
		if(t != NULL)
			delete t;
	}
}

void threadPool::addTask(func f, void* d)
{
	std::unique_lock<std::mutex> m(mtx);
	if (taskNum == taskSize) {				//任务队列满 阻塞等待
		std::cout << "full" << std::endl;
		cv.wait(m, [this]() {return taskNum < taskSize; });
	}

	task* t = new task(f,d,0);
	{
		//添加任务
		list.push_back(t);
		this->taskNum++;
	}

	m.unlock();

	if (taskNum - 1 == 0) {			//如果添加前任务队列为空 唤醒线程取任务
		cv.notify_one();
	}
	
}


void threadPool::shutdown()
{
	shutDown = true;
	std::cout << "shutdown" << std::endl;
	cv.notify_all();
}