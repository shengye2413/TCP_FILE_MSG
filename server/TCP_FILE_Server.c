#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//消息结构体
typedef struct information 
{
    int data;
    char buf[512];
    char str[0];
}information;

//发送文件
int PutFile(information what,int sock_son)
{
    //读取文件
    int fd =open(what.buf,O_RDONLY);
    printf("%s\n",what.buf);
    if(fd==-1)
    {
        perror("错误：");
        return -1;
    }
    char str[512];
    int size;
    //循环把文件写入套接字
    while ((size=read(fd,str,512))>0)
    {
        printf("%d\n",size);
        write(sock_son,str,size);
    }
    close(fd);
    return 0;
}

//接收文件
int GetFile(information what,int sock_son)
{
    //打开或创建文件
    FILE *fd=fopen(what.buf,"w+");
    if (fd==NULL)
    {
        puts("open file failed");
        return -1;
    }

    //循环读取并写入文件
    char buf[512];
    int num;
    while ((num=read(sock_son,buf,512))>0)
    {
        fwrite(buf,num,1,fd);
    }
    puts("file get");

    fclose(fd);
    return 0;
}

int main(int argc ,const char *argv[])
{
    //不按规定链接，退出程序
    if (argc!=3)
    {
        printf("%s [ip] [port]\n",argv[0]);
        exit(0);
    }
    
    //创建套接字描述符
    int sockfd=socket(AF_INET ,SOCK_STREAM,0);
    if(sockfd==-1)
    {
        perror("create socket failed");
        return -1;
    }

    // 设置套接字选项
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //允许套接字绑定到一个之前被使用过的地址
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)); //允许套接字绑定到一个之前被使用过的端口
    
    //设置网络地址
    struct sockaddr_in sock_info;
    sock_info.sin_family        =AF_INET;               //IPV4
    sock_info.sin_port          =htons(atoi(argv[2]));  //端口
    sock_info.sin_addr.s_addr   =inet_addr(argv[1]);    //服务端IP地址

    //为套接字绑定网络地址
    if(bind(sockfd,(struct sockaddr*)&sock_info,sizeof(sock_info))==-1)
    {
        perror("Bind socket failed");
        close(sockfd);
        return -1;
    }

    //进入监听
    if(listen(sockfd,5)==-1)
    {
        perror("listen socket failed");
        close(sockfd);
        return -1;
    }

    //等待客户端
    while (1)
    {
        //获取客户端网络地址
        struct sockaddr_in client_info;
        int size=sizeof(client_info);

        //等待客户端发起请求
        int sock_son=accept(sockfd,(struct sockaddr*)&client_info,&size);
        if(sock_son ==-1)
        {
            //没链接上跳过当前循环
            continue;
        }
        //连接上输出已连接
        printf("%s 连接成功\n",inet_ntoa(client_info.sin_addr));

        //创建进程接收消息
        pid_t pid=fork();
        if(pid==0)
        {
            while (1)
            {
                //读取消息
                information what;
                if(recv(sock_son,&what,512,0)==0) 
                break;
                //判断为消息还是索取文件
                if(what.data==0)
                {
                    if(!strcmp(what.buf,"exit"))
                        break;
                    printf("来自:%s msg:%s\n",inet_ntoa(client_info.sin_addr),what.buf);
                }
                else if(what.data==1)
                {             
                    int put=PutFile(what,sock_son);
                    if(put==-1)
                    puts("put file failed");
                    else if(put==0)
                    puts("put end"); 
                    
                }
                else if(what.data==2)
                {             
                    int put=GetFile(what,sock_son);
                    if(put==-1)
                    puts("get file failed");
                    else if(put==0)
                    puts("get end"); 
                    break;
                }
            }
            printf("客户端：%s退出\n",inet_ntoa(client_info.sin_addr));
            close(sock_son);
            exit(0);
        }
    }
}