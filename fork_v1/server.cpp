#include "server.h"

int main()
{
	Connection connection;
	pid_t pid;
	while(true)
	{
		int fd = connection.getConnection();
		if((pid = fork())< 0)
			std::cerr << "fork error" << endl;
		else if(pid == 0)
		{
			close(connection.sockfd_);
			WebServerGet web_server(fd);
			web_server.dealUser();
				
			exit(0);
		}
		else
			close(fd);
	}
}

