#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
//消息结构体
typedef struct information 
{
    int data;//消息类型，0为发消息，1为下载文件，2为上传文件
    char buf[512];//消息内容
    char str[0];
}information;

information what;

typedef struct File
{
    char buf[512];
    char status;
}File;


//下载文件
void* GetFile(void *sockfd)
{   
    pthread_mutex_t mutex; 
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_lock(&mutex);
    int sockfd_back=*((int *)sockfd);
    printf("%s\n",what.buf);
    //打开或创建文件
    FILE *fd=fopen(what.buf,"w+");
    if (fd==NULL)
    {
        perror("open file failed");
        pthread_exit(0);
    }

    //循环读取并写入文件
    File file;
    int num;

    //ssize_t received = recv(sockfd_back, &file, sizeof(what), 0);

    while (recv(sockfd_back, &file, sizeof(file), 0))
    {
        if(file.status=='f')
        //num=recv(sockfd_back,file.buf,sizeof(file.buf),0);
        {
            num=strlen(file.buf);
            printf("%d\n",num);
            fwrite(file.buf,1,num,fd);
        }
        if(file.status=='e')
        break;
    }
    puts("file get");

    fclose(fd);
    pthread_mutex_unlock(&mutex);
    pthread_exit(0);
}
    
//上传文件
// int PutFile(information what,int sockfd)
// {
//     //打开文件
//     FILE *fd=fopen(what.buf,"r+");
//     if (fd==NULL)
//     {
//         puts("open file failed");
//         return -1;
//     }
    
//     //循环读取写入套接字
//     int num;
//     char buf[512];
//     while ((num=fread(buf,512,1,fd))>0)
//     {
//         printf("%d\n",num);
//         write(sockfd,buf,num);
//     }
//     fclose(fd);
//     return 0;

// }

int main(int argc,const char *argv[])
{
    //不按规定链接，退出程序
    if (argc!=3)
    {
        printf("%s [ip] [port]\n",argv[0]);
        exit(0);
    }

    //创建套接字
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        perror("Create socket failed");
        return -1;
    }

    // 配置服务端的地址信息，用于套接字连接服务器
    struct sockaddr_in server_info;
    server_info.sin_family      = AF_INET;              //IPV4
    server_info.sin_port        = htons(atoi(argv[2])); //端口
    server_info.sin_addr.s_addr = inet_addr(argv[1]);   //服务端IP地址

    //发起请求
    if(connect(sockfd, (struct sockaddr *)&server_info,sizeof(server_info))==-1)
    {
        perror("Connect server failed");//失败返回错误并关闭套接字
        close(sockfd);
        return -1;
    }

    //发送消息
    while (1)
    {
        //定义消息结构体
        printf("类型:");
        scanf("%d",&what.data);
        puts("消息：");
        scanf("%s",what.buf);
        write(sockfd,&what,512);//把消息写入套接字
        //指定条件退出
        if(what.data==0 && strcmp(what.buf,"exit")==0)
        {
            break;
        }
        //当为下载文件时
        if(what.data==1)
        {
            pthread_t pid;
            pthread_create(&pid,NULL,GetFile,&sockfd);
            pthread_join(pid,NULL);

        }
        //当为上传文件时
        // else if(what.data==2)
        // {
        //     int put=PutFile(what,sockfd);
        //     if(put==-1)
        //     puts("put file failed");
        // }
    }
    close(sockfd);
}
