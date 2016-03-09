#ifndef WEB_SERVER_JOB_CONTROL_H_
#define WEB_SERVER_JOB_CONTROL_H_

void listenJob(void*);
void readJob(void*,int);
void writeJob(void *,int);

class WebJob 
{
public:
	virtual void* execute(void * ) = 0;
};

class ListenJob : public WebJob
{
public:
	void * execute(void * server)
	{
		listenJob(server);
		return NULL;
	}
};

class ReadJob : public WebJob
{
	int fd;

public:
	ReadJob(int i):fd(i){}

	void * execute(void * server) override
	{
		readJob(server,fd);
		return NULL;
	}
};


class WriteJob : public WebJob
{
	int fd;
public:
	WriteJob(int i):fd(i){}

	void * execute(void * server) override
	{
		writeJob(server,fd);
		return NULL;
	}
};

inline void jobExecute(void * process, void * args)
{
	WebJob * web_ptr = reinterpret_cast<WebJob*>(process);
	web_ptr->execute(args);
	delete web_ptr;
}

#endif
