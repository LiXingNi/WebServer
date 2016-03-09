#include "thread.h"

#include <stdio.h>
#include <unistd.h>

#define READJOB true
#define WRITEJOB false


void * test(void * arg)
{

	bool * job_category = reinterpret_cast<bool*>(arg);
	bool job_cate = *job_category;

	if(job_cate == READJOB)
		printf("read Job\n");
	else
		printf("write Job\n");
	sleep(1);

	delete job_category;
	return NULL;
}

int main()
{
	Pool * pool = new Pool(3);
	for(int i = 0; i < 10; ++i)
	{
		if(i % 2 == 0)
			pool->addJob(test,reinterpret_cast<void*>(new bool(READJOB)));
		else
			pool->addJob(test,reinterpret_cast<void*>(new bool(WRITEJOB)));
	}
	sleep(6);
	delete pool;
	printf("end main\n") ;
}
