/* Copyright (C) 2001,2004-2005 Bruce Guenter <bruceg@em.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include "alloc.h"
#include "byte.h"
#include "env.h"
#include "fd.h"
#include "fmt.h"
#include "fork.h"
#include "str.h"
#include "substdio.h"

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

#ifndef BUFSIZE
#define BUFSIZE 4096
#endif

#define QQ_DROP_MESSAGE 99

#define MESSAGE_IN 0
#define MESSAGE_OUT 1
#define ENVELOPE_IN 3
#define ENVELOPE_OUT 4
#define QMAILQUEUE_OVERRIDE 5

static const char* binqqargs[2];
static size_t envelope_len = 0;
static size_t message_len = 0;

static char errbuf[SUBSTDIO_OUTSIZE];
static substdio sserr = SUBSTDIO_FDBUF(write,2,errbuf,sizeof(errbuf));
static char pidstr[FMT_ULONG];

struct command
{
  char** argv;
  struct command* next;
};
typedef struct command command;

static void qqf_debug(const char *s1,const char *s2)
{
  if (!env_get("QQF_DEBUG")) return;

  substdio_puts(&sserr,"qmail-qfilter pid ");
  substdio_puts(&sserr,pidstr);
  substdio_puts(&sserr," ");
  substdio_puts(&sserr,s1);
  substdio_puts(&sserr," ");
  substdio_puts(&sserr,s2);
  substdio_puts(&sserr,"\n");
  substdio_flush(&sserr);
}

static void qqf_debug_ulong(const char *s1,unsigned long val)
{
  char strnum[FMT_ULONG];

  byte_zero(strnum,FMT_ULONG);
  fmt_ulong(strnum,val);
  return qqf_debug(s1,strnum);
}

static void die(int exitcode)
{
  qqf_debug_ulong("exit",exitcode);
  exit(exitcode);
}

static void die_nomem(void)    { die(51); }
static void die_write(void)    { die(53); }
static void die_internal(void) { die(81); }
static void die_envelope(void) { die(91); }

static int env_put2_ulong(const char* key, unsigned long val)
{
  char strnum[FMT_ULONG];

  byte_zero(strnum,FMT_ULONG);
  fmt_ulong(strnum,val);
  return env_put2(key,strnum);
}

static size_t parse_sender(const char* envelope)
{
  const char* ptr = envelope;
  char* at;
  size_t len = str_len(envelope);

  if (*ptr != 'F') die_envelope();
  ++ptr;

  env_unset("QMAILNAME");

  if (!*ptr) {
    if (!env_put("QMAILUSER=")) die_nomem();
    if (!env_put("QMAILHOST=")) die_nomem();
    return 2;
  }

  at = strrchr(ptr, '@');
  if (!at) {
    len = str_len(ptr);
    if (!env_put2("QMAILUSER",ptr)) die_nomem();
    if (!env_put("QMAILHOST=")) die_nomem();
  }
  else {
    char *user;
    size_t user_len;

    len = str_len(at);

    user_len = str_len(ptr) - len;
    user = malloc(user_len + 1);
    for (int i = 0; i < user_len; i++)
      user[i] = ptr[i];
    user[user_len] = 0;

    if (!env_put2("QMAILUSER",user)) die_nomem();
    if (!env_put2("QMAILHOST",at+1)) die_nomem();
    ptr = at;
  }

  return ptr + len + 1 - envelope;
}

static void parse_recipients(const char* envelope, int offset)
{
  size_t len = envelope_len - offset;
  const char* ptr = envelope + offset;
  char* buf = malloc(len);
  char* tmp = buf;
  unsigned long count = 0;

  while (ptr < envelope + envelope_len && *ptr == 'T') {
    size_t rcptlen = str_len(++ptr);

    memcpy(tmp, ptr, rcptlen);
    tmp[rcptlen] = '\n';
    tmp += rcptlen + 1;
    ptr += rcptlen + 1;
    ++count;
  }
  *tmp = 0;
  if (!env_put2("QMAILRCPTS",buf)) die_nomem();
  if (!env_put2_ulong("NUMRCPTS", count)) die_nomem();
  free(buf);
}

static void parse_envelope(int fd)
{
  const char* envelope;
  size_t offset;

  envelope = mmap(0, envelope_len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (envelope == MAP_FAILED) die_nomem();
  offset = parse_sender(envelope);
  parse_recipients(envelope, offset);
  munmap((void*)envelope, envelope_len);
}

static int invisible_readwrite_tempfile()
{
  char filename[sizeof(TMPDIR)+19] = TMPDIR "/fixheaders.XXXXXX";
  int fd = mkstemp(filename);

  if (fd == -1) die_write();
  if (unlink(filename) == -1) die_write();

  return fd;
}

static void rewind_fd(int fd)
{
  if (lseek(fd, 0, SEEK_SET) != 0) die_write();
}

static size_t copy_fd_contents_and_close(int fdin, int fdout)
{
  size_t bytes;
  int tmp = invisible_readwrite_tempfile();
  char buf[BUFSIZE];
  substdio ssin = SUBSTDIO_FDBUF(read,fdin,buf,sizeof(buf));
  substdio sstmp = SUBSTDIO_FDBUF(write,tmp,buf,sizeof(buf));

  for (bytes = 0;;) {
    ssize_t r = substdio_get(&ssin,buf,sizeof(buf));

    if (r == -1) die_write();
    if (r == 0)
      break;

    if (substdio_putflush(&sstmp,buf,sizeof(buf)) == -1) die_write();
    bytes += r;
  }

  close(fdin);
  rewind_fd(tmp);
  if (fd_move(fdout,tmp) == -1) die_write();

  return bytes;
}

static command* parse_args_to_linked_list_of_filters(int argc, char* argv[])
{
  command* tail = 0;
  command* head = 0;

  while (argc > 0) {
    command* cmd;
    int end = 0;

    while (end < argc && str_diff(argv[end], "--"))
      ++end;
    if (end == 0) die_internal();
    argv[end] = 0;
    cmd = malloc(sizeof(command));
    cmd->argv = argv;
    cmd->next = 0;
    if (tail)
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

static void invisible_readwrite_tempfileat(int fd)
{
  int tmp;

  close(fd);
  tmp = invisible_readwrite_tempfile();
  if (fd_move(fd,tmp) == -1) die_write();
}

static void move_unless_empty(int src, int dst, const void* reopen,
                              size_t* var)
{
  struct stat st;

  if (fstat(src, &st) != 0) die_internal();
  if (st.st_size > 0) {
    if (fd_move(dst,src) == -1) die_write();
    *var = st.st_size;
    if (reopen) {
      invisible_readwrite_tempfileat(src);
      if (src == ENVELOPE_OUT)
        parse_envelope(ENVELOPE_IN);
    }
  }
  else
    if (!reopen)
      close(src);
  rewind_fd(dst);
}

static char *qq_overridden_by_filter(int fd)
{
  struct stat st;
  char* buf = 0;

  if (fstat(fd, &st) != 0) die_internal();
  if (st.st_size > 0) {
    buf = malloc(st.st_size + 1);
    if (!buf) die_internal();
    if (read(fd, buf, st.st_size) != st.st_size) die_internal();
    buf[st.st_size] = 0;
  }
  close(fd);

  return buf;
}

static void run_filters_in_sequence(const command* first)
{
  const command* c;

  invisible_readwrite_tempfileat(MESSAGE_OUT);
  invisible_readwrite_tempfileat(ENVELOPE_OUT);

  for (c = first; c; c = c->next) {
    pid_t pid;
    int status;

    if (!env_put2_ulong("ENVSIZE", envelope_len)) die_nomem();
    if (!env_put2_ulong("MSGSIZE", message_len)) die_nomem();
    pid = fork();
    if (pid == -1) die_nomem();
    if (pid == 0) {
      execvp(c->argv[0], c->argv);
      die_internal();
    }
    qqf_debug_ulong(c->argv[0],pid);
    if (waitpid(pid, &status, WUNTRACED) == -1) die_internal();
    if (!WIFEXITED(status)) die_internal();
    if (WEXITSTATUS(status))
      die((WEXITSTATUS(status) == QQ_DROP_MESSAGE) ? 0 : WEXITSTATUS(status));
    move_unless_empty(MESSAGE_OUT, MESSAGE_IN, c->next, &message_len);
    move_unless_empty(ENVELOPE_OUT, ENVELOPE_IN, c->next, &envelope_len);
    rewind_fd(QMAILQUEUE_OVERRIDE);
  }
}

static void setup_qqargs(int fd)
{
  if (!binqqargs[0])
    binqqargs[0] = qq_overridden_by_filter(fd);
  if (!binqqargs[0])
    binqqargs[0] = env_get("QQF_QMAILQUEUE");
  if (!binqqargs[0])
    binqqargs[0] = "bin/qmail-queue";
}

int main(int argc, char* argv[])
{
  const command* filters = parse_args_to_linked_list_of_filters(argc-1, argv+1);

  fmt_ulong(pidstr,getpid());

  if (!env_put2_ulong("QMAILPPID", getppid())) die_nomem();
  qqf_debug("ppid",env_get("QMAILPPID"));

  message_len = copy_fd_contents_and_close(0, 0);
  envelope_len = copy_fd_contents_and_close(1, ENVELOPE_IN);
  parse_envelope(ENVELOPE_IN);
  invisible_readwrite_tempfileat(QMAILQUEUE_OVERRIDE);

  run_filters_in_sequence(filters);

  setup_qqargs(QMAILQUEUE_OVERRIDE);
  if (fd_move(1,ENVELOPE_IN) == -1) die_write();
  qqf_debug("execv",binqqargs[0]);
  execv(binqqargs[0], (char**)binqqargs);
  die_internal();
}
