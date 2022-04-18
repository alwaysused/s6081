#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

  int fd[2],pid;
  if (pipe(fd) < 0)
  	exit(1);
  pid = fork();
  if (pid == 0){
  char byte;
  read(fd[0],&byte,1);
  write(fd[1],&byte,1);
  int pidc = getpid();
  printf("%d: received ping\n",pidc);
  exit(0);
  }else{
  char byte;
  write(fd[1],&byte,1);
  wait((int*)0);
  read(fd[0],&byte,1);
  int pidp = getpid();
  printf("%d: received pong\n",pidp);
  }
  exit(0);
}
