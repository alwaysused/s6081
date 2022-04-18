#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

  int fd[2],readpid,writepid;
  int child = 0;
  int factor = 0;
  pipe(fd);
  writepid = fd[1];
  readpid = fd[0];
  int c = fork();
  if (c == 0){
  close(writepid);
    for(;;){
      int cur;
      int length = read(readpid,&cur,4);

      if (length == 0){
        //printf("read 0xxx\n");
        close(readpid);
        close(writepid);
        wait((int*)0);
        exit(0);
      }
      //printf("factor %d read %d from fd %d\n",factor,cur,readpid);
      if (factor == 0 ){
        factor = cur;
        printf("prime %d\n",cur);
      }else{
        if(cur%factor != 0 && child == 0 ){
        int fdd[2];
          pipe(fdd);
          child = fork();
          if (child == 0){
          readpid = fdd[0];
          child = 0;
          factor = 0;
          close(fdd[1]);
          }else{
          close(fdd[0]);
            writepid = fdd[1];
            write(writepid,&cur,4);
            //printf("factor %d write %d to fd %d\n",factor,cur,writepid);
          }

          }else if (cur % factor != 0 && child > 0){
          write(writepid,&cur,4);
                      //printf("factor %d write %d to fd %d\n",factor,cur,writepid);

          }
        }

      }

    }
  else{
    for (int i = 2; i <= 35;i ++){
   write(writepid,&i,4);
  }
  }
  close(writepid);
  close(readpid);

  wait((int*)0);
  exit(0);
}
