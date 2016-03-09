#ifndef WEB_SERVER_BASE_HEADER_H_
#define WEB_SERVER_BASE_HEADER_H_

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
#include <memory>
#include <map>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::vector;
using std::stringstream;
using std::map;
using std::shared_ptr;


#define WORK
#define MAX_LEN 4096 
const string END_FLAG("\r\n");
//const string response_header("Connection: keep-alive\r\nContent-Length: ");
const string response_header("HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: ");
//const string response_length("Content-Length:");

#endif
