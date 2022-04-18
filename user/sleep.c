#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

  if (argc < 2){
  	write(1,"error params \n",1);
  }else{
  	int second = atoi(argv[1]);
  	sleep(second);
  }
  exit(0);
}
