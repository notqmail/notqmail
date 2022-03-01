/* This is free and unencumbered software released into the public * domain. */

#include <bglibs/sysdeps.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

#ifndef BUFSIZE
#define BUFSIZE 4096
#endif

#ifndef QMAIL_QUEUE
#define QMAIL_QUEUE "/var/qmail/bin/qmail-queue"
#endif

#define QQ_OOM 51
#define QQ_WRITE_ERROR 53
#define QQ_INTERNAL 81
#define QQ_BAD_ENV 91

#define QQ_DROP_MSG 99

#define MSGIN 0
#define MSGOUT 1
#define ENVIN 3
#define ENVOUT 4
#define QQFD 5

static const char* qqargv[2];

/* a replacement for setenv(3) for systems that don't have one */
void mysetenv(const char* key, const char* val, size_t vallen)
{
  char* tmp;
  size_t keylen;
  
  keylen = strlen(key);
  tmp = malloc(keylen + 1 + vallen + 1);
  memcpy(tmp, key, keylen);
  tmp[keylen] = '=';
  memcpy(tmp+keylen+1, val, vallen);
  tmp[keylen+1+vallen] = 0;
  if (putenv(tmp) != 0)
    exit(QQ_OOM);
}

void mysetenvu(const char* key, unsigned long val)
{
  char buf[40];
  int i;
  i = sizeof buf;
  buf[--i] = 0;
  do {
    buf[--i] = (val % 10) + '0';
    val /= 10;
  } while (val > 0);
  mysetenv(key, buf + i, sizeof buf - 1 - i);
}

static size_t env_len = 0;
static size_t msg_len = 0;

/* Parse the sender address into user and host portions */
size_t parse_sender(const char* env)
{
  const char* ptr = env;
  char* at;
  size_t len = strlen(env);
  
  if(*ptr != 'F')
    exit(QQ_BAD_ENV);
  ++ptr;
  
  unsetenv("QMAILNAME");
  
  if(!*ptr) {
    if (putenv("QMAILUSER=") != 0
	|| putenv("QMAILHOST=") != 0)
      exit(QQ_OOM);
    return 2;
  }

  at = strrchr(ptr, '@');
  if(!at) {
    len = strlen(ptr);
    mysetenv("QMAILUSER", ptr, len);
    putenv("QMAILHOST=");
  }
  else {
    len = strlen(at);
    mysetenv("QMAILUSER", ptr, at-ptr);
    mysetenv("QMAILHOST", at+1, len-1);
    ptr = at;
  }
  return ptr + len + 1 - env;
}

void parse_rcpts(const char* env, int offset)
{
  size_t len = env_len - offset;
  const char* ptr = env + offset;
  char* buf = malloc(len);
  char* tmp = buf;
  unsigned long count;
  count = 0;
  while(ptr < env + env_len && *ptr == 'T') {
    size_t rcptlen = strlen(++ptr);
    memcpy(tmp, ptr, rcptlen);
    tmp[rcptlen] = '\n';
    tmp += rcptlen + 1;
    ptr += rcptlen + 1;
    ++count;
  }
  *tmp = 0;
  mysetenv("QMAILRCPTS", buf, tmp-buf);
  mysetenvu("NUMRCPTS", count);
  free(buf);
}

void parse_envelope(void)
{
  const char* env;
  size_t offset;
  if ((env = mmap(0, env_len, PROT_READ, MAP_PRIVATE, ENVIN, 0)) == MAP_FAILED)
    exit(QQ_OOM);
  offset = parse_sender(env);
  parse_rcpts(env, offset);
  munmap((void*)env, env_len);
}

/* Create a temporary invisible file opened for read/write */
int mktmpfile()
{
  char filename[sizeof(TMPDIR)+19] = TMPDIR "/fixheaders.XXXXXX";
  
  int fd = mkstemp(filename);
  if(fd == -1)
    exit(QQ_WRITE_ERROR);

  /* The following makes the temporary file disappear immediately on
     program exit. */
  if(unlink(filename) == -1)
    exit(QQ_WRITE_ERROR);
  
  return fd;
}

/* Renumber from one FD to another */
void move_fd(int currfd, int newfd)
{
  if (currfd == newfd)
    return;
  if (dup2(currfd, newfd) != newfd)
    exit(QQ_WRITE_ERROR);
  if (close(currfd) != 0)
    exit(QQ_WRITE_ERROR);
}

/* Copy from one FD to a temporary FD */
void copy_fd(int fdin, int fdout, size_t* var)
{
  unsigned long bytes;
  int tmp = mktmpfile();
  
  /* Copy the message into the temporary file */
  for(bytes = 0;;) {
    char buf[BUFSIZE];
    ssize_t rd = read(fdin, buf, BUFSIZE);
    if(rd == -1)
      exit(QQ_WRITE_ERROR);
    if(rd == 0)
      break;
    if(write(tmp, buf, rd) != rd)
      exit(QQ_WRITE_ERROR);
    bytes += rd;
  }

  close(fdin);
  if (lseek(tmp, 0, SEEK_SET) != 0)
    exit(QQ_WRITE_ERROR);
  move_fd(tmp, fdout);
  *var = bytes;
}

struct command
{
  char** argv;
  struct command* next;
};
typedef struct command command;

/* Split up the command line into a linked list of seperate commands */
command* parse_args(int argc, char* argv[])
{
  command* tail = 0;
  command* head = 0;
  while(argc > 0) {
    command* cmd;
    int end = 0;
    while(end < argc && strcmp(argv[end], "--"))
      ++end;
    if(end == 0)
      exit(QQ_INTERNAL);
    argv[end] = 0;
    cmd = malloc(sizeof(command));
    cmd->argv = argv;
    cmd->next = 0;
    if(tail)
      tail->next = cmd;
    else
      head = cmd;
    tail = cmd;
    ++end;
    argv += end;
    argc -= end;
  }
  return head;
}

static void mktmpfd(int fd)
{
  int tmp;
  close(fd);
  tmp = mktmpfile();
  move_fd(tmp, fd);
}

static void move_unless_empty(int src, int dst, const void* reopen,
			      size_t* var)
{
  struct stat st;
  if (fstat(src, &st) != 0)
    exit(QQ_INTERNAL);
  if (st.st_size > 0) {
    move_fd(src, dst);
    *var = st.st_size;
    if (reopen) {
      mktmpfd(src);
      if (src == ENVOUT)
	parse_envelope();
    }
  }
  else
    if (!reopen)
      close(src);
  if (lseek(dst, 0, SEEK_SET) != 0)
    exit(QQ_WRITE_ERROR);
}

static void read_qqfd(void)
{
  struct stat st;
  char* buf;
  if (fstat(QQFD, &st) != 0)
    exit(QQ_INTERNAL);
  if (st.st_size > 0) {
    if ((buf = malloc(st.st_size + 1)) == 0)
      exit(QQ_INTERNAL);
    if (read(QQFD, buf, st.st_size) != st.st_size)
      exit(QQ_INTERNAL);
    buf[st.st_size] = 0;
    qqargv[0] = buf;
  }
  close(QQFD);
}

/* Run each of the filters in sequence */
void run_filters(const command* first)
{
  const command* c;
  
  mktmpfd(MSGOUT);
  mktmpfd(ENVOUT);

  for(c = first; c; c = c->next) {
    pid_t pid;
    int status;

    mysetenvu("ENVSIZE", env_len);
    mysetenvu("MSGSIZE", msg_len);
    pid = fork();
    if(pid == -1)
      exit(QQ_OOM);
    if(pid == 0) {
      execvp(c->argv[0], c->argv);
      exit(QQ_INTERNAL);
    }
    if(waitpid(pid, &status, WUNTRACED) == -1)
      exit(QQ_INTERNAL);
    if(!WIFEXITED(status))
      exit(QQ_INTERNAL);
    if(WEXITSTATUS(status))
      exit((WEXITSTATUS(status) == QQ_DROP_MSG) ? 0 : WEXITSTATUS(status));
    move_unless_empty(MSGOUT, MSGIN, c->next, &msg_len);
    move_unless_empty(ENVOUT, ENVIN, c->next, &env_len);
    if (lseek(QQFD, 0, SEEK_SET) != 0)
      exit(QQ_WRITE_ERROR);
  }
}

int main(int argc, char* argv[])
{
  const command* filters;
  
  filters = parse_args(argc-1, argv+1);

  if ((qqargv[0] = getenv("QQF_QMAILQUEUE")) == 0)
    qqargv[0] = QMAIL_QUEUE;

  mysetenvu("QMAILPPID", getppid());

  copy_fd(0, 0, &msg_len);
  copy_fd(1, ENVIN, &env_len);
  parse_envelope();
  mktmpfd(QQFD);

  run_filters(filters);

  read_qqfd();
  move_fd(ENVIN, 1);
  execv(qqargv[0], (char**)qqargv);
  return QQ_INTERNAL;
}
