#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "readwrite.h"
#include "exit.h"
#include "scan.h"
#include "fmt.h"
#include "strerr.h"
#include "open.h"
#include "seek.h"
#include "fork.h"

#define FATAL "idedit: fatal: "
#define WARNING "idedit: warning: "

int fd;

void byte(pos,value)
char *pos;
unsigned int value;
{
  unsigned long u;
  unsigned char ch;

  if (pos[scan_ulong(pos,&u)]) return;

  if (seek_set(fd,(seek_pos) u) == -1)
    strerr_die2sys(111,FATAL,"unable to seek: ");

  ch = value;
  if (write(fd,&ch,1) != 1)
    strerr_die2sys(111,FATAL,"unable to write: ");
}

char *args[10];

void run()
{
  int pid;
  int wstat;

  pid = fork();
  if (pid == -1)
    strerr_die2sys(111,FATAL,"unable to fork: ");

  if (pid == 0) {
    execv(*args,args);
    strerr_die4sys(111,WARNING,"unable to run ",*args,": ");
  }

  if (wait_pid(&wstat,pid) != pid)
    strerr_die2sys(111,FATAL,"waitpid surprise");
}

void u(account,group,home,pos0,pos1,pos2,pos3)
char *account;
char *group;
char *home;
char *pos0;
char *pos1;
char *pos2;
char *pos3;
{
  struct passwd *pw;
  unsigned int value;

  pw = getpwnam(account);

  if (!pw && group) {
    args[0] = "add-account";
    args[1] = account;
    args[2] = group;
    args[3] = home;
    args[4] = 0;
    run();
    pw = getpwnam(account);
  }

  if (!pw)
    strerr_die3x(111,FATAL,"unable to find uid for ",account);

  value = pw->pw_uid;
  byte(pos0,value); value >>= 8;
  byte(pos1,value); value >>= 8;
  byte(pos2,value); value >>= 8;
  byte(pos3,value); value >>= 8;
  if (value)
    strerr_die3x(111,FATAL,"excessively large uid for ",account);
}

void g(group,pos0,pos1,pos2,pos3)
char *group;
char *pos0;
char *pos1;
char *pos2;
char *pos3;
{
  struct group *gr;
  unsigned int value;

  gr = getgrnam(group);

  if (!gr) {
    args[0] = "add-group";
    args[1] = group;
    args[2] = 0;
    run();
    gr = getgrnam(group);
  }

  if (!gr)
    strerr_die3x(111,FATAL,"unable to find gid for ",group);

  value = gr->gr_gid;
  byte(pos0,value); value >>= 8;
  byte(pos1,value); value >>= 8;
  byte(pos2,value); value >>= 8;
  byte(pos3,value); value >>= 8;
  if (value)
    strerr_die3x(111,FATAL,"excessively large gid for ",group);
}

void main(argc,argv)
int argc;
char **argv;
{
  if (argc < 42) _exit(100);

  fd = open_write(argv[1]);
  if (fd == -1) strerr_die4sys(111,FATAL,"unable to open ",argv[1],": ");

  g("qmail",argv[34],argv[35],argv[36],argv[37]);
  g("nofiles",argv[38],argv[39],argv[40],argv[41]);

  u("root",(char *) 0,"/",argv[14],argv[15],argv[16],argv[17]);

  u("qmaild","nofiles","/var/qmail",argv[6],argv[7],argv[8],argv[9]);
  u("qmaill","nofiles","/var/qmail",argv[10],argv[11],argv[12],argv[13]);
  u("qmailp","nofiles","/var/qmail",argv[18],argv[19],argv[20],argv[21]);
  u("alias","nofiles","/var/qmail/alias",argv[2],argv[3],argv[4],argv[5]);

  u("qmailq","qmail","/var/qmail",argv[22],argv[23],argv[24],argv[25]);
  u("qmailr","qmail","/var/qmail",argv[26],argv[27],argv[28],argv[29]);
  u("qmails","qmail","/var/qmail",argv[30],argv[31],argv[32],argv[33]);

  _exit(0);
}
