#include "server.h"

void print(const string & msg,pthread_t num)
{
	printf("%s:%ld\n",msg.c_str(),long(num));
}

void print(const string & m1,int num,const string & m2)
{
	printf("%s %d %s\n",m1.c_str(),num,m2.c_str());
}

void * thread_control(void * arg)
{
	Pool * pool = reinterpret_cast<Pool*>(arg);

	while(1)
	{
		pthread_mutex_lock(&pool->queue_lock_);

#ifndef WORK
		print("starting thread",pthread_self());
#endif

		while(pool->queue_q_.empty() && pool->shut_down_ == false)
		{
#ifndef WORK
			print("thread:",pthread_self(),"is waiting");
#endif
			pthread_cond_wait(&pool->queue_cond_,&pool->queue_lock_);

		}

		if(pool->shut_down_ )
		{
			pthread_mutex_unlock(&pool->queue_lock_);
			pthread_exit(NULL);
		}

#ifndef WORK
		print("thread",pthread_self(),"is working");
#endif

		Job * temp = pool->queue_q_.front();
		pool->queue_q_.pop();
		pthread_mutex_unlock(&pool->queue_lock_);

		jobExecute(temp->process_,temp->arg_);

		EpollServer * server = reinterpret_cast<EpollServer*>(temp->arg_);
		delete temp;

		pthread_mutex_lock(&server->server_lock_);
		--server->job_count_;
#ifndef WORK
		cout << "job count:" << server->job_count_ << endl;
#endif
		if(server->job_count_ == 0)
		{
			pthread_mutex_unlock(&server->server_lock_);
			pthread_mutex_lock(&server->server_cond_lock_);
			pthread_mutex_unlock(&server->server_cond_lock_);
			pthread_cond_signal(&server->server_cond_);
		}
		else 
			pthread_mutex_unlock(&server->server_lock_);
	}
}


//---------------- some function -----------------------
void listenJob(void * server)
{
		EpollServer * epoll_server = reinterpret_cast<EpollServer*>(server);
		int connfd;
		struct epoll_event ev;
		ev.events = EPOLLIN;
#ifndef WORK
		print("working listen job",pthread_self());
#endif
		pthread_mutex_lock(&epoll_server->server_lock_);
		while((connfd = epoll_server->connect_.getConnection()) > 0)
		{
			ev.data.fd = connfd;
			epoll_server->inputEvent(connfd,ev);
#ifndef WORK
			printf("add fd %d\n",connfd);
#endif
		}
		pthread_mutex_unlock(&epoll_server->server_lock_);
}

void readJob(void * server,int fd)
{
	EpollServer * epoll_server = reinterpret_cast<EpollServer*>(server);
	
	struct epoll_event ev;
	ev.events = EPOLLOUT;
	ev.data.fd = fd;


#ifndef WORK
	printf("%d is reading by : %ld\n",fd,long(pthread_self()));
#endif
	string info_path = RequestControl::requestRead(fd);
#ifndef WORK
	print("read job finish",pthread_self());
#endif

	pthread_mutex_lock(&epoll_server->server_lock_);
	if(info_path.empty()) //read error and not keep_alive
	{
		epoll_server->removeEvent(fd,NULL); // 从可读监听队列删除
		close(fd);  //关闭套接字
#ifndef WORK
		printf("read job : remove Event\n");
#endif
	}
	else
	{
		epoll_server->write_job_[fd] = info_path;
		epoll_server->modifyEvent(fd,ev);
#ifndef WORK
		printf("read job : modify Event\n");
#endif
	}
#ifndef WORK
	printf("%ld finish read job\n",long(pthread_self()));
#endif
	pthread_mutex_unlock(&epoll_server->server_lock_);
}

void writeJob(void * server,int fd)
{
	EpollServer * epoll_server = reinterpret_cast<EpollServer*>(server);

	int stat;
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = fd;

#ifndef WORK
	printf("%d working write job  %ld\n",fd,long(pthread_self()));
#endif

	pthread_mutex_lock(&epoll_server->server_lock_);

	map<int,string>::iterator it = epoll_server->write_job_.find(fd);
	if(it == epoll_server->write_job_.end())
	{
		std::cerr << "do not have job" << endl;
		pthread_mutex_unlock(&epoll_server->server_lock_);
		return;
	}
	string tmp = epoll_server->write_job_[fd];
	epoll_server->write_job_.erase(it);

	pthread_mutex_unlock(&epoll_server->server_lock_);

	stat = RequestControl::requestWrite(fd,tmp);


#ifndef WORK
	print("write job finish  ",pthread_self());
#endif
	pthread_mutex_lock(&epoll_server->server_lock_);

	if(stat < 0)
	{
		epoll_server->removeEvent(fd,NULL); // 从可写监听队列删除
		close(fd);
	}
	else
	{
		epoll_server->modifyEvent(fd,ev);
	}
#ifndef WORK
	printf("finish write job %ld\n",long(pthread_self()));
#endif
	pthread_mutex_unlock(&epoll_server->server_lock_);
}
