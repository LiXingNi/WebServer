#ifndef WEB_SERVER_THREAD_H_
#define WEB_SERVER_THREAD_H_

#include <pthread.h>
#include <iostream>
#include <queue>

using	std::cout;
using	std::cin;
using	std::endl;
using	std::queue; 

#define READJOB	true
#define WRITEJOB false

void jobExecute(void*,void*);

class Pool;
void * thread_control(void * arg);
class EpollServer;

class Job
{
	friend class Pool;
	friend void* thread_control(void*);
	Job(void* p ,void * a):
		process_(p),arg_(a){}
	void *process_;
	void * arg_;
};

class Pool
{
	friend void* thread_control(void*);
	friend class EpollServer;
	queue<Job*> queue_q_;
	pthread_mutex_t queue_lock_;
	pthread_cond_t queue_cond_;
	pthread_t * threadid_;
	int max_thread_num_;
	bool shut_down_;

public:
	Pool(int num):max_thread_num_(num),shut_down_(false)
	{
		//初始化锁与条件变量
		pthread_mutex_init(&queue_lock_,NULL);
		pthread_cond_init(&queue_cond_,NULL);


		threadid_ = new pthread_t[max_thread_num_];

		for(int i = 0;i != max_thread_num_;++i)
			pthread_create(&threadid_[i],NULL,thread_control,this);
	}

	void addJob(void* process, void * arg)
	{
		Job * temp = new Job(process,arg);

		pthread_mutex_lock(&queue_lock_);
		queue_q_.push(temp);
		pthread_mutex_unlock(&queue_lock_);
		pthread_cond_signal(&queue_cond_);
	}

	~Pool()
	{
		shut_down_ = true;

		pthread_cond_broadcast(&queue_cond_);
		for(int i = 0;i != max_thread_num_;++i)
			pthread_join(threadid_[i],NULL);
		delete []threadid_;

		Job * temp = NULL;
		while(!queue_q_.empty())
		{
			temp = queue_q_.front();
			queue_q_.pop();
			delete temp;
			temp = 0;
		}

		pthread_mutex_destroy(&queue_lock_);
		pthread_cond_destroy(&queue_cond_);
		cout << "destroy all" << endl;
	}
};

#endif

