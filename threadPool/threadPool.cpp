#include "threadPool.h"

threadPool::threadPool(int taskSize,int threadNum){
	this->taskSize = taskSize;
	this->ThreadNum = threadNum;
	taskNum = 0;
	shutDown = false;
	for (int i = 0; i < threadNum; ++i) {
		
		workers.push_back(std::thread([this]() {

			while (true) {

				//thread execute 
				{
					std::unique_lock<std::mutex> m(mtx);
					while (taskNum == 0 && !shutDown) {
						cv.wait(m);
					}
					
					//pool close
					if (shutDown) {
						break;
					}

					if (taskNum == 0)
						continue;

					task* temp = list.front();
					list.pop_front();
					taskNum--;
					m.unlock();
					if (taskNum + 1 == this->taskSize || taskNum == 0) {
						cv.notify_all();	//notify add task
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
		std::cout << it->get_id() << std::endl;
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
	if (taskNum == taskSize) {				//queue full
		std::cout << "full" << std::endl;
		cv.wait(m, [this]() {return taskNum < taskSize; });
	}

	task* t = new task(f,d);
	{
		//add task
		list.push_back(t);
		this->taskNum++;
	}

	m.unlock();

	if (taskNum - 1 == 0) {			//aften not null 
		cv.notify_one();
	}
	
}


void threadPool::shutdown()
{
	shutDown = true;
	cv.notify_all();
}