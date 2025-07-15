#include <iostream>
#include "sayHi.h"
#include "server.h"


int main(int argc,char* argv[]) {
	if (argc < 2) {
        LOG_ERROR << "Usage: " << argv[0] << " <port>";
        return 1;
    }
    int port = std::atoi(argv[1]);  // תΪ int
	MuduoEventLoop loop;
	Server ser(&loop,MuduoInetAddress(muduo::StringArg("127.0.0.1"),port));
	ser.m_startServer();
	std::cout << "Hello, Server!" << std::endl;

	loop.loop();
	return 0;
}