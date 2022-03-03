#include "server.h"


int make_child(process_data_t*parr ,int process_num){
    pid_t pid;
    int fds[2];//这个是与子进程之间的管道
    int ret,i;
    for(i=0;i<process_num;i++)//创建多个子进程
    {
        ret=socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
        ERROR_CHECK(ret,-1,"socketpair");
        pid=fork();
        if(0==pid)
        {
            close(fds[1]);
            child_handle(fds[0]);//一定要让子进程child_handle内exit
        }
        close(fds[0]);
        //父进程通过fds[1]和子进程进行通信，和pipe的区别是fds[1]即可以读也可以写
        parr[i].pid=pid;
        parr[i].fd=fds[1];
        parr[i].busy=0;//默认子进程是非忙碌状态
#ifdef DEBUG
        printf("p_manage[%d].fd=%d\n",i,parr[i].fd);
#endif
    }
}

int child_handle(int fd){
    int new_fd,ret,exit_flag;//子进程要接收一个new_fd,通过new_fd给对应的客户端发文件
    char flag=1;
    int tmp_len;
    ptrain_t pt=(train_t *)malloc(sizeof(train_t));//申请一片空间先接命令
    while(1){
        recv_fd(fd,&new_fd,&exit_flag);//接收任务
        if(exit_flag)
            {
                printf("child %d will exit\n",getpid());
                exit(0);
            }
        while(1)
        {
            bzero(pt,sizeof(pt));
            ret=recv(new_fd,&tmp_len,sizeof(int),0);
            
            if(tmp_len == 0 || ret == 0)
            {
                printf("client exit !\n");
                close(new_fd);
                write(fd,&flag,sizeof(flag));//通知父进程完成任务
                free(pt);
                break;
                // exit(0);
            }
            recvn(new_fd,pt->buf,tmp_len);//收火车车厢
            printf("gets task,new_fd=%d,%s\n",new_fd,pt->buf);
            
            if(strncmp("cd", pt->buf, 2) == 0)
            {
                do_cd(new_fd,pt);
            }else if(strncmp("ls", pt->buf, 2) == 0)
            {
                do_ls(new_fd,pt);
            }else if( strncmp("puts",pt->buf, 4)== 0)
            {
                do_puts(new_fd,pt);
            }else if( strncmp("gets", pt->buf, 4)== 0)
            {
                do_gets(new_fd,pt);

            }else if( strncmp("remove", pt->buf, 6)== 0)
            {
                do_remove(new_fd,pt);

            }else if(strncmp("pwd", pt->buf, 3) == 0) 
            {
                do_pwd(new_fd,pt);

            }else 
            {
                continue;
            }
        }
    }
}