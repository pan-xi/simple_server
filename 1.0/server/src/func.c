#include "server.h"

int sendn(socket_t fd_send, char* send_buf, int len)
{
	int sum = 0 ;
	int nsend ;
	while(sum < len)
	{
		nsend = send(fd_send, send_buf + sum, len - sum, 0);
		sum += nsend ;
	}
	return sum ;
}

int recvn(socket_t fd_recv,char* recv_buf, int len)
{
	int sum = 0 ;
	int nrecv ;
	while(sum < len)
	{
		nrecv = recv(fd_recv, &recv_buf[ sum], len - sum, 0);
		sum += nrecv ;
	}
	recv_buf[sum] = '\0';
	return sum ;
}

 void do_cd(socket_t new_fd,ptrain_t pt) 
{
	char dir[128]= "";
	sscanf(pt->buf+3, "%s", dir);
	chdir(dir);
	getcwd(dir, 128);
	int len = strlen(dir);
	send(new_fd, &len, sizeof(int), 0);
	sendn(new_fd, dir, strlen(dir));
}

char* file_type(mode_t md)
{
	if(S_ISREG(md))
	{
		return "-";	
	}else if(S_ISDIR(md))
	{
		return "d";
	}else if(S_ISFIFO(md))
	{
		return "p";
	}else 
	{
		return "o" ;
	}
}

void do_ls(socket_t new_fd,ptrain_t pt) 
{
	DIR* pdir = opendir("./");
	if(pdir == NULL)
	{
		int flag = -1 ;
		send(new_fd, &flag, sizeof(int), 0);
	}else
	{
		struct dirent* mydir ;
		int len ;
		while( (mydir = readdir(pdir)) != NULL)//读取目录
		{
			if(strncmp(mydir->d_name, ".", 1) == 0 || strncmp(mydir->d_name,"..", 2) == 0)
			{
				continue ;
			}
			struct stat mystat;
			bzero(&mystat, sizeof(stat));
			stat(mydir->d_name, &mystat);//获取某个文件的信息
			bzero(pt->buf, 1024);
			sprintf(pt->buf, "%-2s%-20s %10ldB", file_type(mystat.st_mode),mydir->d_name, mystat.st_size );
			len =  strlen(pt->buf);
			send(new_fd, &len, sizeof(int), 0);//发送数据给客户端
			sendn(new_fd, pt->buf, len);
		}
		len = 0 ;
		send(new_fd, &len, sizeof(int), 0);
	}
}
void do_puts(socket_t new_fd,ptrain_t pt) //puts a.txt b.txt
{
	char file_name[256];
	int file_pos = 5 ;
	//puts file,文件名在file_name中
	while(bzero(file_name, 256), sscanf(pt->buf + file_pos,"%s", file_name) == 1)
	{
		file_pos += strlen(file_name) + 1 ;//为了puts后面跟多个文件名
		if(download(new_fd, file_name) == 0)
		{
			printf("file -> %s download success \n", file_name);
		}else
		{
			printf("file -> %s download failed \n", file_name);
		}
	}
}

//服务器传文件给客户端
void do_gets(socket_t new_fd,ptrain_t pt) 
{
	char file_name[256];
	int file_pos = 5 ;
	while(bzero(file_name, 256),sscanf(pt->buf + file_pos,"%s", file_name ) == 1)
	{
		file_pos += strlen(file_name) + 1; 
		if(upload(new_fd, file_name) == 0)
		{
			printf(" file-> %s upload success\n", file_name);
		}else 
		{
			printf(" file-> %s upload failed\n", file_name);
		}

	}

}

int readn(int fd_read, char* read_buf, int len)
{
	int sum = 0 ;
	int nread ;
	while(sum < len)
	{
		nread = read(fd_read, &read_buf[ sum], len - sum);
		if(nread == 0)
		{
			break ;
		}
		sum += nread ;
	}
	read_buf[sum] = '\0';
	return sum ;
		
}

int upload(socket_t fd_up, char* file_name)
{
	int fd_file = open(file_name, O_RDONLY);
	if(fd_file == -1)
	{
		return -1 ;
	}
	char *read_buf = (char*)malloc(8 * 1024);
	bzero(read_buf, 8 * 1024);
	int nread ;
	while(1)
	{
		nread = readn(fd_file, read_buf, 8192);
		if(nread < 8192)
		{
			send(fd_up, &nread, sizeof(int), 0);
			sendn(fd_up, read_buf, nread);
			break ;
		}else
		{
			
			send(fd_up, &nread, sizeof(int), 0);
			sendn(fd_up, read_buf, nread);
		}
	}
	int flag = 0 ;
	send(fd_up, &flag, sizeof(flag), 0);
	close(fd_file);
	return 0 ;
}

int download(socket_t fd_down, char* file_name)
{
	int fd_file = open(file_name, O_WRONLY|O_CREAT,0666 );
	if(fd_file == -1)
	{
		return -1 ;
	}
	char* write_buf = (char*)malloc(8192);
	bzero(write_buf, 8192);
	int nwrite ;
	while(1)
	{
		recv(fd_down, &nwrite, sizeof(int), 0);
		if(nwrite == 0)
		{
			break ;
		}
		recvn(fd_down, write_buf, nwrite);
		writen(fd_file, write_buf, nwrite);
	}
	close(fd_file);
	return 0 ;
}

int writen(int fd_write, char* write_buf, int len)
{
	int sum = 0 ;
	int nwrite ;
	while(sum < len)
	{
		nwrite = write(fd_write, write_buf + sum, len - sum);
		sum += nwrite ;
	}
	return sum ;

}

//使用脚本的rm命令来删除文件，目前只能删除普通文件
void do_remove(socket_t new_fd,ptrain_t pt)// remove file 
{
	char cmd[256] ="" ;
	sprintf(cmd, "rm -f %s",pt->buf + 7);
	system(cmd);
	bzero(pt->buf, BUF_SIZE);
	sprintf(pt->buf, " removed");
	int len = strlen(pt->buf);
	send(new_fd, &len, sizeof(int),0);
	sendn(new_fd, pt->buf, len);
}

void do_pwd(socket_t new_fd,ptrain_t pt )
{
	bzero(pt->buf, BUF_SIZE);
	getcwd(pt->buf, BUF_SIZE);
	int len = strlen(pt->buf);
	send(new_fd, &len, sizeof(int), 0);
	sendn(new_fd, pt->buf, len);

}