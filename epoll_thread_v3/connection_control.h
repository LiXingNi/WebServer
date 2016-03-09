#ifndef WEB_SERVER_CONNECTION_H_
#define WEB_SERVER_CONNECTION_H_

#include "base_header.h"

#define LISTENQ 1024	
class ConnectionControl
{
	inline void CheckConnection(int fd,const string & msg)
	{
		if(fd < 0)
			std::cerr << msg << endl;
	}
public:
	int sockfd_;

	ConnectionControl(int port = 9999)
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
		//CheckConnection(connfd,"accept error");
		return connfd;
	}
	
};

#endif
