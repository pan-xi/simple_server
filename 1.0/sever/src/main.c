#include "server.h"

int exit_pipefd[2];//退出管道
void sig_exit_func(int signum)
{//父进程一手掌握两端
    write(exit_pipefd[1],&signum,1);//异步拉起同步,信号是异步机制
}

int main(int argc, char* argv[]){
    if(argc!=3)
    {
        printf("./process_pool_server CONFIG PROCESS_NUM\n");
        return -1;
    }
    char ip[16],port[8];
    get_value(argv[1],ip,port);//从文件中拿ip，port
    socket_t fd_server = socket_server(ip, port);//socket,bind,listen
    if(fd_server==-1){
        printf("socket wrong");
        return -1;
    }
    int process_num=atoi(argv[2]);
    //parr用来管理所有的子进程信息的
    p_manage parr=(process_data_t *)calloc(process_num,sizeof(process_data_t));
    make_child(parr,process_num);
    //初始化epoll，父进程既要监控fd_server，也要监控每一个子进程的管道对端
    int epfd=epoll_create(1);
    int ret=epoll_add(epfd,fd_server);
    SELFFUNC_ERR_CHECK(ret,"epoll_add");
    int i;
    for(i=0;i<process_num;i++)
    {//为每个端口添加epoll监视
        ret=epoll_add(epfd,parr[i].fd);
        SELFFUNC_ERR_CHECK(ret,"epoll_add");
    }
    //退出做准备
    pipe(exit_pipefd);
    signal(SIGINT,sig_exit_func);
    ret=epoll_add(epfd,exit_pipefd[0]);
    SELFFUNC_ERR_CHECK(ret,"epoll_add");
    int new_fd,j,non;//non用来记录是否有进程接任务
    int ready_fd_num;//就绪的描述符数量
    int conn_num;//连接的客户端
    struct epoll_event *evs=(struct epoll_event *)calloc(process_num+2,sizeof(struct epoll_event));
    struct sockaddr_in client_addr;
    pQue_t pque=(Que_t*)malloc(sizeof(Que_t));
    que_init(pque);
    pNode_t pnew;//队列任务新结点
    char flag,log_buf[128];
    int fd_open=open("log",O_RDWR | O_CREAT,0755);//简单打印日志
    while(1){
        non=0;
        ready_fd_num=epoll_wait(epfd,evs,process_num+2,-1);//last 0 会立即返回,-1 将不确定,也有说法说是永久阻塞
        for(i=0;i<ready_fd_num;i++){
            //父进程等待是否有客户端连接，一旦有客户端连接，accept接收，得到new_fd
            //new_fd传递给非忙碌的子进程，把它标记为忙碌
            if(evs[i].data.fd==fd_server)
            {
                bzero(&client_addr,sizeof(client_addr));
                socklen_t addr_len=sizeof(client_addr);
                new_fd=accept(fd_server,(struct sockaddr*)&client_addr,&addr_len);
                non=1;//接过来后把non设1
                bzero(log_buf, 128);
                sprintf(log_buf, "%d connect\n",++conn_num );
                printf("%s",log_buf);
                printf("%s %d is connect\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
                for(j=0;j<process_num;j++)//找到非忙碌的子进程
                {
                    if(0==parr[j].busy)
                    {
                        send_fd(parr[j].fd,new_fd,0);//发任务给子进程
                        parr[j].busy=1;//对应的子进程被标记为忙碌
                        non=0;//有进程接任务
                        printf("child %d is busy\n",parr[j].pid);
                        break;
                    }
                }
                if(0==non){
                    close(new_fd);//找到空闲子进程，关闭new_fd
                }
                else{
                     pnew=(pNode_t)calloc(1,sizeof(Node_t));
                     pnew->new_fd=new_fd;
                     pthread_mutex_lock(&pque->mutex);//加锁，放队列，解锁
                     que_set(pque,pnew);//任务队列
                     pthread_mutex_unlock(&pque->mutex);
                }
            }
              //如果收到子进程的通知，把对应子进程标记为非忙碌
            for(j=0;j<process_num;j++)
            {
                if(evs[i].data.fd==parr[j].fd)//说明对应的子进程发通知了
                {
                    read(parr[j].fd,&flag,sizeof(flag));//把数据读出来
                    parr[j].busy=0;
                    printf("child %d is not busy\n",parr[j].pid);
                    break;
                }
            }
            if(evs[i].data.fd==exit_pipefd[0])//程序收到信号要退出了
            {
                for(j=0;j<process_num;j++)
                {
                    send_fd(parr[j].fd,0,1);//发退出通知给子进程
                }
                for(j=0;j<process_num;j++)
                {
                    wait(NULL);//回收每个子进程,本身是随机等一个
                }
                close(fd_server);
                return 0;
            }
        }
    }
    return 0;
}