#ifndef __FTP_H__
#define __FTP_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#define BUF_SIZE 8*1024//缓冲区大小
#define ERROR_CHECK(ret,retval,func_name) {if(ret==retval) {printf("errno=%d,",errno);fflush(stdout);perror(func_name);return -1;}}
#define SELFFUNC_ERR_CHECK(ret,func_name) {if(ret!=0) {printf("%s failed\n",func_name);return -1;}}

typedef int socket_t ;

typedef struct{//管理子进程
    pid_t pid;//每个子进程的pid
    int fd;//子进程的对端
    short busy;//子进程是否忙碌，如果是0代表非忙碌，1代码忙碌
}process_data_t,*p_manage;

typedef struct tag_node{//任务队列数据结构
    int new_fd;
    struct tag_node *pNext;
}Node_t,*pNode_t;

typedef struct{//管理任务队列
    pNode_t que_head,que_tail;//队列头队列尾
    int que_size;//当前队列大小
    pthread_mutex_t mutex;
}Que_t,*pQue_t;
//不要交换data_len和buf的定义顺序
typedef struct{
    int data_len;//存放buf，也就是火车车厢中数据长度
    char buf[BUF_SIZE];//火车车厢长度
}train_t,*ptrain_t;
//从config获取ip，port
int get_value(char*path, char* ip, char* port);
void get_target(char*arg,FILE*fp_conf);
void get_from_file(char*line,char*arg);
//绑定ip port，并listen
int socket_server(char* ip, char* port);
//创建进程池
int make_child(process_data_t*parr ,int process_num);
//epoll注册
int epoll_add(int epfd,int fd);
//子进程行为
int child_handle(int fd);
//初始化队列和锁
void que_init(pQue_t pq);
//放入队列
void que_set(pQue_t pq,pNode_t pnew);
//拿出队列
void que_get(pQue_t pq,pNode_t *p);
//发描述符
int send_fd(int sfd,int fd,int exit_flag);
//收描述符
int recv_fd(int sfd,int *fd,int *exit_flag);
//循环发送
int sendn(socket_t fd_send, char* send_buf, int len);
//循化接收
int recvn(socket_t fd_recv,char* recv_buf, int len);
void do_cd(socket_t new_fd,ptrain_t pt);
void do_ls(socket_t new_fd,ptrain_t pt) ;
void do_puts(socket_t new_fd,ptrain_t pt);
void do_gets(socket_t new_fd,ptrain_t pt) ;
int readn(int fd_read, char* read_buf, int len);
int upload(socket_t fd_up, char* file_name);
int download(socket_t fd_down, char* file_name);
int writen(int fd_write, char* write_buf, int len);
void do_remove(socket_t new_fd,ptrain_t pt);
void do_pwd(socket_t new_fd,ptrain_t pt );
#endif