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

class Pool;
void * thread_control(void * arg);

class Job
{
	friend class Pool;
	friend void* thread_control(void*);
	Job(void*(*p)(void *),void * a):
		process_(p),arg_(a){}
	void *(*process_)(void *);
	void * arg_;
};

class Pool
{
	friend void* thread_control(void*);
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

	void addJob(void *(*process)(void*),void * arg)
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

void * thread_control(void * arg)
{
	Pool * pool = reinterpret_cast<Pool*>(arg);

	while(1)
	{
		pthread_mutex_lock(&pool->queue_lock_);

#ifndef WORK
		cout << "starting thread: " << pthread_self() << endl;
#endif

		while(pool->queue_q_.empty() && pool->shut_down_ == false)
		{
#ifndef WORK
		cout << "thread: " << pthread_self()  << " is waiting"<< endl;
#endif
		pthread_cond_wait(&pool->queue_cond_,&pool->queue_lock_);

		}

		if(pool->shut_down_ )
		{
			pthread_mutex_unlock(&pool->queue_lock_);
			pthread_exit(NULL);
		}

#ifndef WORK
		cout << "thread: " << pthread_self()  << " is working"<< endl;
#endif

		Job * temp = pool->queue_q_.front();
		pool->queue_q_.pop();
		pthread_mutex_unlock(&pool->queue_lock_);

		(*(temp->process_))(temp->arg_);
		delete temp;
		temp = 0;
	}
}

#endif

