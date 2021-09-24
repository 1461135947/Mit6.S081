#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int 
main(int argc, char *argv[])
{
  if(argc!=1 ){
      fprintf(2, "Usage: pingpong\n");
    exit(1);
  }
  int pipe1[2],pipe2[2];
  char buff[64];

//初始化管道
  pipe(pipe1);
  pipe(pipe2);
  int pid=fork();

//父进程  pipe1写 pipe2读 
  if(pid>0){
      close(pipe1[0]);
      close(pipe2[1]);
      write(pipe1[1],"\0",1);
      if(read(pipe2[0],buff,sizeof(buff))>0){
          printf("%d: received pong\n",getpid());
      }else
      {
          fprintf(2, "pingpong error\n");
          exit(1);
      }

// 子进程 pipe1读 pipe2写
  }else if(pid==0){
      close(pipe1[1]);
      close(pipe2[0]);
      if(read(pipe1[0],buff,sizeof(buff))>0){
          printf("%d: received ping\n",getpid());
      }else
      {
          fprintf(2, "pingpong error\n");
          exit(1);
      }
      write(pipe2[1],"\0",1);
  }else{
      fprintf(2, "fork error\n");
  }
  exit(0);
}
