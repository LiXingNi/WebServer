#ifndef WEB_SERVER_DEAL_MSG_H_
#define WEB_SERVER_DEAL_MSG_H_

#include "base_header.h"

class IOControl
{
public:
	/*
	static int readLine(int fd,string & msg)
	{
		msg.clear();	
		char c;
		int n;
		while((n = read(fd,&c,1)) > 0)
		{
			msg += c;
			if(c == '\n')
				break;
		}
		return n;
	}
	*/

	static ssize_t readLine(int fd,string & msg)
	{
		char buff[MAX_LEN];
		size_t max_len = MAX_LEN;
		ssize_t n,rc;
		char c , *ptr = buff;

		for(n = 1; n < MAX_LEN;n++)
		{
again:
			if((rc = read(fd,&c,1)) == 1)
			{
				*ptr++ = c;
				if(c == '\n')
					break;
			}
			else if(rc == 0)
			{
				*ptr == 0;
				return (n-1);
			}
			else
			{
				if(errno == EINTR)
					goto again;
				return -1;
			}
		}
		*ptr = 0;
		msg = string(buff);
		return n;
		
	}

	static int readHeader(int fd,vector<string> & msg_vec)
	{
		string line;
		int n;
		while((n = readLine(fd,line)) > 0 && line != END_FLAG)	
		{
#ifndef WORK
			cout << "READ HEADER: " << n << endl;
#endif
			msg_vec.push_back(line);
		}
		return n;
	}

	/*
	static int writeN(int fd,const char * msg,size_t len)
	{
		size_t left_bit = len;
		size_t location = 0;
		int n;

		while((n = write(fd,msg + location,left_bit)) > 0)
		{
			left_bit -= n;
			if(left_bit == 0) break;
			location += n;
		}
		cout << "left_bit: " << left_bit << endl;
		return n;
	}
	*/

	static int writeN(int fd,const char * msg,size_t len)
	{
		size_t nleft = len;
		ssize_t nwritten;
		const char * ptr = msg;

		while(nleft > 0)
		{
			if((nwritten = write(fd,ptr,nleft)) <=  0)
			{
				cout << "nwritten: " << nwritten << endl;
				cout << errno << endl;
				if(nwritten < 0 && errno == EINTR)
					nwritten = 0;
				else
					return -1;
			}
			nleft -= nwritten;
			ptr += nwritten;
		}
		return len;
	}
};

#endif
