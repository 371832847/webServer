#ifndef EPOLLER_H
#define EPOLLER_H


#include<vector>
#include<sys/epoll.h>
#include<assert.h>
#include<unistd.h>



class epoller {
public:
	explicit epoller(int maxEvent = 1024);
	~epoller();

	bool add_fd(int fd, uint32_t events);

	bool mod_fd(int fd, uint32_t events);

	bool del_fd(int fd);

	int wait(int timeout_ms = -1);

	int get_event_fd(size_t i) const;

	uint32_t getEvents(size_t i) const;

private:
	int epollFd;
	std::vector<struct epoll_event> events;

};


#endif