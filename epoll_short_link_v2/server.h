#ifndef WEB_SERVER_SERVER_H_
#define WEB_SERVER_SERVER_H_

#include "connection_control.h"
#include "request_control.h"

class EpollServer
{
	ConnectionControl connect_;
	map<int,string> write_job_;
	int epfd_;
	struct epoll_event events_[2048];
	const int Max_wait_events_;

	inline void inputEvent(int fd,struct epoll_event & ev)
	{
		if(epoll_ctl(epfd_,EPOLL_CTL_ADD,fd,&ev) < 0)
			std::cerr << "inputEvent error" << endl;
	}

	inline void removeEvent(int fd,struct epoll_event & ev)
	{
		if(epoll_ctl(epfd_,EPOLL_CTL_DEL,fd,&ev) < 0)
			std::cerr << "removeEvent error" << endl;
	}

	inline void modifyEvent(int fd, struct epoll_event & ev) 
	{
		if(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) < 0)
			std::cerr << "modEvent error" << endl;
	}

public:
	EpollServer(int max_total_events = 2048,int max_wait_events = 1024): Max_wait_events_(max_wait_events){
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
	}

	void execute()
	{
		while(true)		
		{
			int nfds = epoll_wait(epfd_,events_,Max_wait_events_,-1);
			for(int n = 0; n < nfds;++n)
			{
				if(events_[n].data.fd == connect_.sockfd_)
				{
					int connfd;
					//读取所有新的链接并加入可读列表
					while((connfd = connect_.getConnection()) > 0)
					{
						struct epoll_event ev;
						ev.events = EPOLLIN;
						ev.data.fd = connfd;
						inputEvent(connfd,ev);
					}
				}
				else if(events_[n].events & EPOLLIN) //可读
				{
					int fd = events_[n].data.fd;
					struct epoll_event ev = events_[n];

					string info_path = RequestControl::requestRead(fd);

					if(info_path.empty()) //read error and not keep_alive
					{
						removeEvent(fd,ev); // 从可读监听队列删除
						close(fd);  //关闭套接字
						continue;
					}

					write_job_[fd] = info_path;

					ev.events = EPOLLOUT;
					modifyEvent(fd,ev);  //加入可写监听队列
				} 
				else if(events_[n].events & EPOLLOUT) // 可写
				{

					int fd = events_[n].data.fd;
					int stat;
					struct epoll_event ev = events_[n];

					map<int,string>::iterator it = write_job_.find(fd);
					if(it == write_job_.end())
						std::cerr << "do not have job" << endl;

					string tmp = write_job_[fd];

					stat = RequestControl::requestWrite(fd,write_job_[fd]);
					write_job_.erase(it);
					removeEvent(fd,ev); // 从可写监听队列删除
					close(fd);
				}
			}
		}
	}

};

#endif
