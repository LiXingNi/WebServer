#include "server.h"

int main()
{
	EpollServer * server = new EpollServer();
	server->execute();
	delete server;
}

