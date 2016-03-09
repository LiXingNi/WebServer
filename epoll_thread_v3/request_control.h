#ifndef WEB_SERVER_REQUEST_CONTROL_H_
#define WEB_SERVER_REQUEST_CONTROL_H_
#include "base_header.h"
#include "IO_control.h"

class RequestControl 
{
private:
	static string parseHeader(vector<string>& header)
	{
		string path;
		for(auto line_request : header)
		{
			stringstream strm(line_request);
			string head;
			strm >> head;
			if(head == "GET") 
			{
				strm >> path;
				if(path == "/")
					path = "page/index.html";
				else
					path = "page" + path;
				//deal ?
				path = path.substr(0,path.find_first_of('?'));
				break;
			}
			else continue;
		}
		return path;
	}

	static int sendHeader(int fd,int fileno,const string & path)
	{
		//get file_len
		struct stat stat_buf;
		if(fstat(fileno,&stat_buf) < 0)
			std::cerr << "get file len error " << path << endl;
		//getHeader
		string response(response_header);
		stringstream strm;
		strm << static_cast<int>(stat_buf.st_size);
		response += strm.str();
		response += "\r\n\r\n";
		return IOControl::writeN(fd,response.c_str(),response.size());
	}

public:
	static string requestRead(int fd)
	{
		vector<string> header;
#ifndef WORK
		cout << "come in read header : " << long(pthread_self())<< endl;
#endif
		int n = IOControl::readHeader(fd,header);
#ifndef WORK
		cout << "read header ok : " << long(pthread_self())<< endl;
		cout << header.size() << endl;
		for(string & line : header)
			cout << line << endl;
#endif
		if(n < 0)
		{
			cout << "*******************" << endl;
			for(auto s : header)
				cout << s << endl;
			cout << "*******************" << endl;
			return string("");
		}
		return parseHeader(header);
	}

	static int requestWrite(int fd,const string & path)
	{
		int n,wn;
		char buffer[MAX_LEN];
		int fileno = open(path.c_str(),O_RDONLY);
		wn = sendHeader(fd,fileno,path);
		if(wn < 0)
		{
			cout << "send header wrong " << endl;
			return wn;
		}

		while((n = read(fileno,buffer,MAX_LEN)) > 0)
			wn = IOControl::writeN(fd,buffer,n);
		close(fileno);
		return wn;
	}
};
#endif
