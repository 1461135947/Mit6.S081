#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
// 父进程在fd1写在fd2读;子进程与之相反
void primes(int fd1[2],int fd2[2]){
    char buff[64];
    // 关掉管道fd1写端,fd2读端
    close(fd1[1]);
    close(fd2[0]);
    // 第一次读将数据打印;后续的读将内容转发
    if( read(fd1[0],buff,sizeof(buff))==0){
        exit(0);
    }
    printf("prime %s\n",buff,getpid());
    

    // 创建子进程并且将数据转发给子进程;父进程读pipe_fd1,写pipe_fd2;子进程与之相反
    int pipe_fd1[2],pipe_fd2[2];
    pipe(pipe_fd1);
    pipe(pipe_fd2);
    int pid=fork();
    //父进程  pipe_fd写
    if(pid>0){
        close(pipe_fd1[0]);
        close(pipe_fd2[1]);
       // 通知父进程进行下一轮读写
        fprintf(fd2[1],"1");
        // 转发数据
        while (read(fd1[0],buff,sizeof(buff)))
        {
            char ch[64];
            write(pipe_fd1[1],buff,sizeof(buff));
            // 接收子进程反馈
            read(pipe_fd2[0],ch,sizeof(ch));
            // 通知父进程进行下一轮读写
            fprintf(fd2[1],"1");
        }
        // 没有数据读以后关闭fd的读端和pipe_fd的写端
        close(fd1[0]);
        close(fd2[1]);
        close(pipe_fd1[1]);
        close(pipe_fd2[0]);
        
        wait(&pid);
    //子进程 pipe_fd读
    }else if(pid==0){
        close(fd1[0]);
        close(fd2[1]);
        primes(pipe_fd1,pipe_fd2);
    }else{
        fprintf(2, "fork error\n");
    }
    exit(0);

}
int is_prime(int val){
    for(int i=2;i<val;i++){
        if(val%i==0){
            return -1;
        }
    }
    return 0;
}
int main(int argc, char *argv[])
{
    if(argc!=1 ){
        fprintf(2, "Usage: primes\n");
        exit(1);
    }
    int fd1[2],fd2[2];
    char buff[64];

    //初始化管道
    pipe(fd1);
    pipe(fd2);
    int pid=fork();
    //父进程  循环将质数写入管道
    if(pid>0){
        close(fd1[0]);
        close(fd2[1]);
        for(int i=2;i<35;i++){
            if(is_prime(i)>=0){
                fprintf(fd1[1],"%d",i);
                // 等待子进程反馈进行下一轮读取
                read(fd2[0],buff,64);
            }
        } 
        close(fd1[1]);
        close(fd2[0]);
        wait(&pid);
    // 子进程做prime的逻辑
    }else if(pid==0){
        primes(fd1,fd2);
    }else{
        fprintf(2, "fork error\n");
    }
    exit(0);
}
