#include "client.h"
#include <csignal>
#include <fcntl.h>

int pipefd[2];
MuduoEventLoop* loopPtr;

void signalHandler(int signum) {
    char byte=1;
    write(pipefd[1],&byte,1);
    if(loopPtr!=nullptr){
        loopPtr->quit();
    }
}

int main(int argc, char* argv[]) {

    if (pipe(pipefd) == -1) {
        LOG_ERROR << "Failed to create pipe";
        return 1;
    }
    fcntl(pipefd[1],F_SETFL,O_NONBLOCK);
   
   
     
    MuduoEventLoop loop;
    loopPtr = &loop;

    MuduoInetAddress serverAddr("127.0.0.1", 8000);

    Client client(&loop, serverAddr, pipefd[0]); // Pass the read end of the pipe to the client
   
    client.m_connect();

    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);  // 清空信号屏蔽集
    sa.sa_flags = 0;           // 不设置任何特殊标志
    
    // 可选：在信号处理期间阻塞其他信号
    // sigaddset(&sa.sa_mask, SIGTERM);  // 如果需要的话
    
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        perror("sigaction failed");
        return 1;
    }


    loop.loop();
	return 0;
}