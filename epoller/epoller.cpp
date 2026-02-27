#include"epoller.h"

epoller::epoller(int maxEvent):epollFd(epoll_create(100000)),events(maxEvent)
{
	assert(epollFd >= 0 && events.size() > 0);
}

epoller::~epoller() {
	close(epollFd);
}

bool epoller::add_fd(int fd, uint32_t events)
{
	if(fd < 0)
		return false;

	epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = events;

	return 0 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
}

bool epoller::mod_fd(int fd, uint32_t events)
{
	if(fd < 0)
		return false;

	epoll_event ev = { 0 };
	ev.data.fd = fd;
	ev.events = events;
	return 0 == epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
}

bool epoller::del_fd(int fd)
{
	if(fd < 0)
		return false;
	epoll_event ev = { 0 };
	return 0 == epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ev);
}

int epoller::wait(int timeoutMs)
{
	return epoll_wait(epollFd,&events[0],static_cast<int>(events.size()),timeoutMs);
}

int epoller::get_event_fd(size_t i) const
{
	assert(i < events.size() && i >= 0);
	return events[i].data.fd;
}

uint32_t epoller::get_events(size_t i) const
{
	assert(i < events.size() && i >= 0);
	return events[i].events;
}
