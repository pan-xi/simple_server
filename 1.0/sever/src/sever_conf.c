#include "server.h"

void get_from_file(char*line,char*arg){
    char *p=strchr(line, '=');
    if(p == NULL)
	{
		printf("your file no \"=\"");
        exit(-1);
	}
    strcpy(arg,p+1);
}

void get_target(char*arg,FILE*fp_conf){
    char line[128];
    bzero(line,128);//初始化line
    fgets(line, 128, fp_conf) ;
    line[strlen(line)-1] = '\0' ;
    get_from_file(line,arg);
#ifdef DEBUG
    printf("%s \n",arg);
#endif
}

int get_value(char*path, char* ip, char* port){//从文件获取ip port
    FILE*fp_conf=fopen(path,"r");
    ERROR_CHECK(fp_conf,NULL,"fopen");
    get_target(ip,fp_conf);
    get_target(port,fp_conf);
    fclose(fp_conf);
    return 0;
}