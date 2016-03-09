#ifndef WEB_SERVER_SERVER_H_
#define WEB_SERVER_SERVER_H_

#include "connection_control.h"
#include "request_control.h"
#include "thread.h"
#include "job_control.h"


class WebJob;
class ListenJob;
class ReadJob;

void listenJob(void * );
void readJob(void*,int);
void writeJob(void*,int);

class EpollServer
{
	friend void listenJob(void *);
	friend void readJob(void*,int);
	friend void writeJob(void*,int);
	friend void * thread_control(void*);
	

	ConnectionControl connect_;
	map<int,string> write_job_;
	int epfd_;
	struct epoll_event events_[2048];
	const int Max_wait_events_;
	int job_count_;

	pthread_mutex_t server_lock_;
	pthread_cond_t server_cond_;
	pthread_mutex_t server_cond_lock_;

	Pool pool_; //为了实现线程共享，此时的 pool_所指对象必须是分配在堆上的



	inline void inputEvent(int fd,struct epoll_event & ev)
	{
		if(epoll_ctl(epfd_,EPOLL_CTL_ADD,fd,&ev) < 0)
			std::cerr << "inputEvent error : " << pthread_self()  << endl;
	}

	inline void removeEvent(int fd,void *)
	{
		if(epoll_ctl(epfd_,EPOLL_CTL_DEL,fd,NULL) < 0)
			std::cerr << "removeEvent error" << pthread_self()  << endl;
	}

	inline void modifyEvent(int fd, struct epoll_event & ev) 
	{
		if(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) < 0)
			std::cerr << "modEvent error" << pthread_self()  << endl;
	}

public:
	EpollServer(int thread_num = 100,int max_total_events = 2048,int max_wait_events = 1024): Max_wait_events_(max_wait_events),pool_(thread_num){
		//忽略 SIGPIPE 信号
		signal(SIGPIPE,SIG_IGN);

		//将 listenfd 设为非阻塞模式
		int flag;
		if((flag = fcntl(connect_.sockfd_,F_GETFL,0))< 0)
			std::cerr << "fcntl error" << endl;
		if(fcntl(connect_.sockfd_,F_SETFL,flag | O_NONBLOCK) < 0)
			std::cerr << "set fcntl error" << endl;

		epfd_ = epoll_create(max_total_events);

		struct epoll_event ev;
		ev.data.fd = connect_.sockfd_;
		ev.events  = EPOLLIN | EPOLLET;
		
		inputEvent(connect_.sockfd_,ev);

		//init thread_mutex and thread_cond
		pthread_mutex_init(&server_lock_,NULL);
		pthread_mutex_init(&server_cond_lock_,NULL);
		pthread_cond_init(&server_cond_,NULL);
		pthread_mutex_lock(&server_cond_lock_);
		job_count_ = 0;
	}

	void execute()
	{
		while(true)		
		{
			int nfds = epoll_wait(epfd_,events_,Max_wait_events_,-1);
			job_count_ = nfds;
			for(int n = 0; n < nfds;++n)
			{
				if(events_[n].data.fd == connect_.sockfd_)
				{
					WebJob* job_ptr = new ListenJob();
					pool_.addJob(job_ptr,this);
				}
				else if(events_[n].events & EPOLLIN) //可读
				{
					int fd = events_[n].data.fd;
					WebJob* job_ptr = new ReadJob(fd);
					pool_.addJob(job_ptr,this);
				} 
				else if(events_[n].events & EPOLLOUT) // 可写
				{
					int fd = events_[n].data.fd;
					WebJob* job_ptr = new WriteJob(fd);
					pool_.addJob(job_ptr,this);
				}

				//lock wait
			}
			while(job_count_ != 0)
				pthread_cond_wait(&server_cond_, &server_cond_lock_);
		}
	}


	~EpollServer()
	{
		pthread_mutex_destroy(&server_lock_);
	}

};

#endif
