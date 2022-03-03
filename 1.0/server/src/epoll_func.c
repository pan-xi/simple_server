#include "server.h"


int socket_server(char* ip, char* port){
    int fd_server ;
	struct sockaddr_in server_addr ;
	fd_server = socket(AF_INET, SOCK_STREAM, 0);//初始化一个网络描述符，对应了一个缓冲区
	ERROR_CHECK(fd_server,-1,"socket");
	memset(&server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family=AF_INET;//代表要进行ipv4通信
    server_addr.sin_addr.s_addr=inet_addr(ip);//把ip的点分十进制转为网络字节序
    server_addr.sin_port=htons(atoi(port));//把端口转为网络字节序
	int reuse = 1 ,ret;
	int buf_num = BUF_SIZE;
    ret=setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR,  (void*)&reuse, sizeof(int)) ;//允许重用本地地址和端口
    ERROR_CHECK(ret,-1,"setsockopt1");
    ret=setsockopt(fd_server, SOL_SOCKET, SO_SNDBUF,  (void*)&buf_num, sizeof(int));//发送缓冲区大小
    ERROR_CHECK(ret,-1,"setsockopt2");
    ret=setsockopt(fd_server, SOL_SOCKET, SO_RCVBUF,  (void*)&buf_num, sizeof(int));//接收缓冲区大小
    ERROR_CHECK(ret,-1,"setsockopt3");
    ret=bind(fd_server, (struct sockaddr*)&server_addr, sizeof(server_addr) );//绑定
    ERROR_CHECK(ret,-1,"bind");
    ret=listen(fd_server, 5);
    ERROR_CHECK(ret,-1,"listen");
    return fd_server ;
}

int epoll_add(int epfd,int fd)
{
    struct epoll_event event;
    event.events=EPOLLIN;//读事件
    event.data.fd=fd;
    int ret=epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
    ERROR_CHECK(ret,-1,"epoll_ctl");
    return 0;
}