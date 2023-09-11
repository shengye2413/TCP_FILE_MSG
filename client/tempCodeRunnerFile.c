    {
                //当为下载文件时 
                //if(what.data==1)
                {
                    int get=GetFile(what,sockfd);
                    if(get==-1)
                    puts("get file failed");   
                    exit(0);
                }
                //当为上传文件时
                //else if(what.data==2)
                // {
                //     int put=PutFile(what,sockfd);
                //     if(put==-1)
                //     puts("put file failed");
                // }
            }