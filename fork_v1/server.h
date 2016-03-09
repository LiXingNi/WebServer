#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <signal.h>

#include <cstring>
#include <unistd.h>


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::vector;
using std::stringstream;


#define LISTENQ	7
#define MAX_LEN 4096 
const string END_FLAG("\r\n");

class Connection
{
	inline void CheckConnection(int fd,const string & msg)
	{
		if(fd < 0)
			std::cerr << msg << endl;
	}
public:
	int sockfd_;

	Connection(int port = 9999)
	{
		sockfd_ = socket(AF_INET,SOCK_STREAM,0);
		CheckConnection(sockfd_,"socket error");
		sockaddr_in servaddr;
		bzero(&servaddr,sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

		//set reuseaddr
		int reuse = 1;
		socklen_t len = sizeof(reuse);
		if(setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,(void*)&reuse,len) < 0)
			std::cerr << "reuse error" << endl;
		
		bind(sockfd_,(sockaddr*)&servaddr,sizeof(servaddr));
		listen(sockfd_,LISTENQ);
	}

	inline int getConnection()
	{
		int connfd = accept(sockfd_,NULL,NULL);
		CheckConnection(connfd,"accept error");
		return connfd;
	}
	
};

class DealMsg
{
public:
	static string readLine(int fd,int & stat)
	{
		string msg;
		char c;
		int n;
		while((n = read(fd,&c,1)) > 0)
		{
			msg += c;
			if(c == '\n')
				break;
		}
		stat = n;
		return msg;
	}

	static vector<string> readHeader(int fd,int &stat)
	{
		vector<string> msg_vec;	
		string line;
		int n;
		while((line = readLine(fd,n),n) > 0 && line != END_FLAG)	
		{
			msg_vec.push_back(line);
		}
		stat = n;
		return msg_vec;
	}

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

		if(left_bit == 0)
			return 1;
		else
			return -1;
	}
};

class WebServer
{
protected:
	int connfd_;
public:
	WebServer(int fd):connfd_(fd){signal(SIGPIPE,SIG_IGN);}
	virtual void dealHeader(){};
	void dealUser()
	{
		dealHeader();
		close(connfd_);
	}
};

class WebServerGet : public WebServer
{
	string response_header;
	string response_length;
public:
	WebServerGet(int fd):WebServer(fd),response_header("HTTP/1.1 200 OK\r\nContent-Length:"),response_length("Content-Length:"){}
	char buffer[MAX_LEN];

	void sendHeader(int fileno,const string & path)
	{
		//get file_len
		struct stat stat_buf;
		if(fstat(fileno,&stat_buf) < 0)
			std::cerr << "get file len error: " << path << endl;
		//getHeader
		string response(response_header);
		stringstream strm;
		strm << static_cast<int>(stat_buf.st_size);
		response += strm.str() + "\r\n\r\n";
		DealMsg::writeN(connfd_,response.c_str(),response.size());
	}

	void dealGet(const string & msg)
	{
		stringstream strm(msg);
		string head,path;
		strm >> head;
		if(head != "GET")	return;
		strm >> path;
		if(path == "/")
			path = "page/index.html";
		else
			path = "page" + path;
		//deal ?
		path = path.substr(0,path.find_first_of('?'));
		int fileno = open(path.c_str(),O_RDONLY);
		sendHeader(fileno,path);
		int n;
		while((n = read(fileno,buffer,MAX_LEN)) > 0)
			DealMsg::writeN(connfd_,buffer,n);
	}
public:
	void dealHeader()
	{
		int n;
		vector<string> header = DealMsg::readHeader(connfd_,n);
		for(auto s : header)
			dealGet(s);
	}
};

class EpollSever
{
	Connection connect_;

}
