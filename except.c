#include "fork.h"
#include "strerr.h"
#include "wait.h"
#include "error.h"
#include "exit.h"

#define FATAL "except: fatal: "

void main(argc,argv)
int argc;
char **argv;
{
  int pid;
  int wstat;

  if (!argv[1])
    strerr_die1x(100,"except: usage: except program [ arg ... ]");

  pid = fork();
  if (pid == -1)
    strerr_die2sys(111,FATAL,"unable to fork: ");
  if (pid == 0) {
    execvp(argv[1],argv + 1);
    if (error_temp(errno)) _exit(111);
    _exit(100);
  }

  if (wait_pid(&wstat,pid) == -1)
    strerr_die2x(111,FATAL,"wait failed");
  if (wait_crashed(wstat))
    strerr_die2x(111,FATAL,"child crashed");
  switch(wait_exitcode(wstat)) {
    case 0: _exit(100);
    case 111: strerr_die2x(111,FATAL,"temporary child error");
    default: _exit(0);
  }
}
