#include <sys/types.h>
#include <sys/stat.h>
#include "readwrite.h"
#include "sig.h"
#include "env.h"
#include "byte.h"
#include "exit.h"
#include "fork.h"
#include "open.h"
#include "wait.h"
#include "lock.h"
#include "seek.h"
#include "substdio.h"
#include "getln.h"
#include "subfd.h"
#include "sgetopt.h"
#include "alloc.h"
#include "error.h"
#include "stralloc.h"
#include "fmt.h"
#include "str.h"
#include "now.h"
#include "case.h"
#include "quote.h"
#include "qmail.h"
#include "slurpclose.h"
#include "myctime.h"
#include "gfrom.h"
#include "auto_patrn.h"

void err(s) char *s; { substdio_putsflush(subfderr,s); }
void soft() { _exit(111); }
void hard() { _exit(100); }

void temp_childcrashed() { err("Aack, child crashed. (#4.3.0)\n"); soft(); }
void temp_rewind() { err("Unable to rewind message. (#4.3.0)\n"); soft(); }
void temp_fork() { err("Unable to fork. (#4.3.0)\n"); soft(); }
void temp_read() { err("Error while reading message. (#4.3.0)\n"); soft(); }
void temp_write() { err("Error while writing message. (#4.3.0)\n"); soft(); }
void temp_child() { err("Temporary error in forwarding message. (#4.3.0)\n"); soft(); }
void temp_maildirtimeout() { err("Timeout on maildir delivery. (#4.3.0)\n"); soft(); }
void temp_maildir() { err("Temporary error on maildir delivery. (#4.3.0)\n"); soft(); }
void temp_nomaildir() { err("Unable to chdir to maildir. (#4.2.1)\n"); soft(); }
void temp_open(fn) char *fn; { err("Unable to open "); err(fn); err(". (#4.2.1)\n"); soft(); }

void temp_blankline() { err("Uh-oh: first line of .qmail file is blank. (#4.2.1)\n"); soft(); }
void temp_fofile() { err("Uh-oh: .qmail has file delivery but has x bit set. (#4.7.0)\n"); soft(); }
void temp_foprog() { err("Uh-oh: .qmail has prog delivery but has x bit set. (#4.7.0)\n"); soft(); }
void temp_nomem() { err("Out of memory. (#4.3.0)\n"); soft(); }
void temp_chdir() { err("Unable to switch to home directory. (#4.3.0)\n"); soft(); }
void temp_homestat() { err("Unable to stat home directory. (#4.3.0)\n"); soft(); }
void temp_homesticky() { err("Home directory is sticky: user is editing his .qmail file. (#4.2.1)\n"); soft(); }
void temp_homewritable() { err("Uh-oh: home directory is writable. (#4.7.0)\n"); soft(); }
void temp_qmwritable() { err("Uh-oh: .qmail file is writable. (#4.7.0)\n"); soft(); }
void temp_nfsqmail() { err("Temporary error trying to open .qmail file. (#4.3.0)\n"); soft(); }
void temp_denyqmail() { err("Permission error trying to open .qmail file. (#4.3.0)\n"); soft(); }
void temp_slowlock() { err("File has been locked for 30 seconds straight. (#4.3.0)\n"); soft(); }

void bounce_childperm() { err("Permanent error in forwarding message. (#5.2.4)\n"); hard(); }
void bounce_loop() { err("This message is looping: it already has my Delivered-To line. (#5.4.6)\n"); hard(); }
void bounce_ext() { err("Sorry, no mailbox here by that name. (#5.1.1)\n"); hard(); }
void usage() { err("qmail-local: usage: qmail-local [ -nN ] user homedir local dash ext domain sender aliasempty\n"); hard(); }

void warn_homesticky() { err("Warning: home directory is sticky.\n"); }

int flagdoit;
int flag99;

char *user;
char *homedir;
char *local;
char *dash;
char *ext;
char *host;
char *sender;
char *aliasempty;

stralloc dashext = {0};
stralloc ufline = {0};
stralloc rpline = {0};
stralloc envrecip = {0};
stralloc dtline = {0};
stralloc qme = {0};
stralloc ueo = {0};
stralloc cmds = {0};
stralloc messline = {0};
stralloc foo = {0};

char buf[1024];
char outbuf[1024];

/* child process */

char fntmptph[80 + FMT_ULONG * 2];
char fnnewtph[80 + FMT_ULONG * 2];
void tryunlinktmp() { unlink(fntmptph); }
void sigalrm() { tryunlinktmp(); _exit(3); }

void maildir_child(dir)
char *dir;
{
 unsigned long pid;
 unsigned long time;
 char host[64];
 char *s;
 int loop;
 struct stat st;
 int fd;
 substdio ss;
 substdio ssout;

 sig_alarmcatch(sigalrm);
 if (chdir(dir) == -1) { if (error_temp(errno)) _exit(1); _exit(2); }
 pid = getpid();
 host[0] = 0;
 gethostname(host,sizeof(host));
 for (loop = 0;;++loop)
  {
   time = now();
   s = fntmptph;
   s += fmt_str(s,"tmp/");
   s += fmt_ulong(s,time); *s++ = '.';
   s += fmt_ulong(s,pid); *s++ = '.';
   s += fmt_strn(s,host,sizeof(host)); *s++ = 0;
   if (stat(fntmptph,&st) == -1) if (errno == error_noent) break;
   /* really should never get to this point */
   if (loop == 2) _exit(1);
   sleep(2);
  }
 str_copy(fnnewtph,fntmptph);
 byte_copy(fnnewtph,3,"new");

 alarm(86400);
 fd = open_excl(fntmptph);
 if (fd == -1) _exit(1);

 substdio_fdbuf(&ss,read,0,buf,sizeof(buf));
 substdio_fdbuf(&ssout,write,fd,outbuf,sizeof(outbuf));
 if (substdio_put(&ssout,rpline.s,rpline.len) == -1) goto fail;
 if (substdio_put(&ssout,dtline.s,dtline.len) == -1) goto fail;

 switch(substdio_copy(&ssout,&ss))
  {
   case -2: tryunlinktmp(); _exit(4);
   case -3: goto fail;
  }

 if (substdio_flush(&ssout) == -1) goto fail;
 if (fsync(fd) == -1) goto fail;
 if (close(fd) == -1) goto fail; /* NFS dorks */

 if (link(fntmptph,fnnewtph) == -1) goto fail;
   /* if it was error_exist, almost certainly successful; i hate NFS */
 tryunlinktmp(); _exit(0);

 fail: tryunlinktmp(); _exit(1);
}

/* end child process */

void maildir(fn)
char *fn;
{
 int child;
 int wstat;

 if (seek_begin(0) == -1) temp_rewind();

 switch(child = fork())
  {
   case -1:
     temp_fork();
   case 0:
     maildir_child(fn);
     soft();
  }

 wait_pid(&wstat,child);
 if (wait_crashed(wstat))
   temp_childcrashed();
 switch(wait_exitcode(wstat))
  {
   case 0: break;
   case 2: temp_nomaildir();
   case 3: temp_maildirtimeout();
   case 4: temp_read();
   default: temp_maildir();
  }
}

void slowlock() { temp_slowlock(); }

void mailfile(fn)
char *fn;
{
 int fd;
 substdio ss;
 substdio ssout;
 int match;
 seek_pos pos;
 int flaglocked;

 if (seek_begin(0) == -1) temp_rewind();

 fd = open_append(fn);
 if (fd == -1) temp_open(fn);

 sig_alarmcatch(slowlock);
 alarm(30);
 flaglocked = (lock_ex(fd) != -1);
 alarm(0);
 sig_alarmdefault();

 seek_end(fd);
 pos = seek_cur(fd);

 substdio_fdbuf(&ss,read,0,buf,sizeof(buf));
 substdio_fdbuf(&ssout,write,fd,outbuf,sizeof(outbuf));
 if (substdio_put(&ssout,ufline.s,ufline.len)) goto writeerrs;
 if (substdio_put(&ssout,rpline.s,rpline.len)) goto writeerrs;
 if (substdio_put(&ssout,dtline.s,dtline.len)) goto writeerrs;
 for (;;)
  {
   if (getln(&ss,&messline,&match,'\n') != 0) 
    { if (flaglocked) seek_trunc(fd,pos); close(fd); temp_read(); }
   if (!match && !messline.len) break;
   if (gfrom(messline.s,messline.len))
     if (substdio_bput(&ssout,">",1)) goto writeerrs;
   if (substdio_bput(&ssout,messline.s,messline.len)) goto writeerrs;
   if (!match)
    {
     if (substdio_bputs(&ssout,"\n")) goto writeerrs;
     break;
    }
  }
 if (substdio_bputs(&ssout,"\n")) goto writeerrs;
 if (substdio_flush(&ssout)) goto writeerrs;
 if (fsync(fd) == -1) goto writeerrs;
 close(fd);
 return;

 writeerrs:
 if (flaglocked) seek_trunc(fd,pos);
 close(fd);
 temp_write();
}

void mailprogram(prog)
char *prog;
{
 int child;
 char *(args[4]);
 int wstat;

 if (seek_begin(0) == -1) temp_rewind();

 switch(child = fork())
  {
   case -1:
     temp_fork();
   case 0:
     args[0] = "sh"; args[1] = "-c"; args[2] = prog; args[3] = 0;
     sig_pipedefault();
     execvp(*args,args);
     if (errno == error_txtbsy) { err("Text busy. (#4.3.0)\n"); soft(); }
     if (errno == error_nomem) { err("Out of memory. (#4.3.0)\n"); soft(); }
     if (errno == error_io) { err("I/O error. (#4.3.0)\n"); soft(); }
     if (error_temp(errno)) { err("Temporary error. (#4.3.0)\n"); soft(); }
     err("Unable to execute "); err(*args); err(" (#5.2.4)\n");
     hard();
  }

 wait_pid(&wstat,child);
 if (wait_crashed(wstat))
   temp_childcrashed();
 switch(wait_exitcode(wstat))
  {
   case 100:
   case 64: case 65: case 70: case 76: case 77: case 78: case 112: hard();
   case 0: break;
   case 99: flag99 = 1; break;
   default: soft();
  }
}

unsigned long mailforward_qp = 0;

void mailforward(recips)
char **recips;
{
 struct qmail qqt;
 substdio ss;
 int match;

 if (seek_begin(0) == -1) temp_rewind();
 substdio_fdbuf(&ss,read,0,buf,sizeof(buf));

 if (qmail_open(&qqt) == -1) temp_fork();
 mailforward_qp = qmail_qp(&qqt);
 qmail_put(&qqt,dtline.s,dtline.len);
 do
  {
   if (getln(&ss,&messline,&match,'\n') != 0) { qmail_fail(&qqt); break; }
   qmail_put(&qqt,messline.s,messline.len);
  }
 while (match);
 qmail_from(&qqt,ueo.s);
 while (*recips) qmail_to(&qqt,*recips++);
 switch(qmail_close(&qqt))
  {
   case QMAIL_TOOLONG: bounce_childperm();
   case QMAIL_READ: temp_read();
   case 0: return;
   default: temp_child();
  }
}

void bouncexf()
{
 int match;
 substdio ss;

 if (seek_begin(0) == -1) temp_rewind();
 substdio_fdbuf(&ss,read,0,buf,sizeof(buf));
 for (;;)
  {
   if (getln(&ss,&messline,&match,'\n') != 0) temp_read();
   if (!match) break;
   if (messline.len <= 1)
     break;
   if (messline.len == dtline.len)
     if (!str_diffn(messline.s,dtline.s,dtline.len))
       bounce_loop();
  }
}

void checkhome()
{
 struct stat st;

 if (stat(".",&st) == -1) temp_homestat();
 if (st.st_mode & auto_patrn) temp_homewritable();
 if (st.st_mode & 01000)
   if (flagdoit) temp_homesticky(); else warn_homesticky();
}

int qmeox(dashowner)
char *dashowner;
{
 struct stat st;

 if (!stralloc_copys(&qme,".qmail")) temp_nomem();
 if (!stralloc_cat(&qme,&dashext)) temp_nomem();
 if (!stralloc_cats(&qme,dashowner)) temp_nomem();
 if (!stralloc_0(&qme)) temp_nomem();
 if (stat(qme.s,&st) == -1)
  {
   if (error_temp(errno)) temp_nfsqmail();
   return -1;
  }
 return 0;
}

int qmeopen(cutable)
int *cutable;
{
 int fd;
 struct stat st;
 int i;

 i = dashext.len;
 for (;;)
  {
   if (!stralloc_copys(&qme,".qmail")) temp_nomem();
   if (!stralloc_catb(&qme,dashext.s,i)) temp_nomem();
   if (i < dashext.len) if (!stralloc_cats(&qme,"-default")) temp_nomem();
   if (!stralloc_0(&qme)) temp_nomem();
   fd = open_read(qme.s);
   if (fd == -1)
    {
     if (error_temp(errno)) temp_nfsqmail();
     if (errno == error_perm) temp_denyqmail();
     if (errno == error_acces) temp_denyqmail();
    }
   else
    {
     if (fstat(fd,&st) == -1) temp_nfsqmail();
     if ((st.st_mode & S_IFMT) == S_IFREG)
      {
       if (st.st_mode & auto_patrn) temp_qmwritable();
       *cutable = !!(st.st_mode & 0100);
       return fd;
      }
     close(fd);
    }
   if (!i) return -1;
   do
     if (dashext.s[--i] == '-') break;
   while (i);
  }
}

unsigned long count_file = 0;
unsigned long count_forward = 0;
unsigned long count_program = 0;
char count_buf[FMT_ULONG];

void count_print()
{
 substdio_puts(subfdoutsmall,"did ");
 substdio_put(subfdoutsmall,count_buf,fmt_ulong(count_buf,count_file));
 substdio_puts(subfdoutsmall,"+");
 substdio_put(subfdoutsmall,count_buf,fmt_ulong(count_buf,count_forward));
 substdio_puts(subfdoutsmall,"+");
 substdio_put(subfdoutsmall,count_buf,fmt_ulong(count_buf,count_program));
 substdio_puts(subfdoutsmall,"\n");
 if (mailforward_qp)
  {
   substdio_puts(subfdoutsmall,"qp ");
   substdio_put(subfdoutsmall,count_buf,fmt_ulong(count_buf,mailforward_qp));
   substdio_puts(subfdoutsmall,"\n");
  }
 substdio_flush(subfdoutsmall);
}

void sayit(type,cmd,len)
char *type;
char *cmd;
int len;
{
 substdio_puts(subfdoutsmall,type);
 substdio_put(subfdoutsmall,cmd,len);
 substdio_putsflush(subfdoutsmall,"\n");
}

void main(argc,argv)
int argc;
char **argv;
{
 int opt;
 int i;
 int j;
 int k;
 int fd;
 int numforward;
 char **recips;
 datetime_sec starttime;
 int flagforwardonly;
 char *extx;

 umask(077);
 sig_pipeignore();

 if (!env_init()) temp_nomem();

 flagdoit = 1;
 while ((opt = getopt(argc,argv,"nN")) != opteof)
   switch(opt)
    {
     case 'n': flagdoit = 0; break;
     case 'N': flagdoit = 1; break;
     case '?':
     default:
       hard();
    }
 argc -= optind;
 argv += optind;

 if (!(user = *argv++)) usage();
 if (!(homedir = *argv++)) usage();
 if (!(local = *argv++)) usage();
 if (!(dash = *argv++)) usage();
 if (!(ext = *argv++)) usage();
 if (!(host = *argv++)) usage();
 if (!(sender = *argv++)) usage();
 if (!(aliasempty = *argv++)) usage();
 if (*argv) usage();

 if (homedir[0] != '/') usage();
 if (chdir(homedir) == -1) temp_chdir();
 checkhome();

 if (!env_put2("HOST",host)) temp_nomem();
 if (!env_put2("HOME",homedir)) temp_nomem();
 if (!env_put2("USER",user)) temp_nomem();
 if (!env_put2("LOCAL",local)) temp_nomem();

 if (!stralloc_copys(&envrecip,local)) temp_nomem();
 if (!stralloc_cats(&envrecip,"@")) temp_nomem();
 if (!stralloc_cats(&envrecip,host)) temp_nomem();

 if (!stralloc_copy(&foo,&envrecip)) temp_nomem();
 if (!stralloc_0(&foo)) temp_nomem();
 if (!env_put2("RECIPIENT",foo.s)) temp_nomem();

 if (!stralloc_copys(&dtline,"Delivered-To: ")) temp_nomem();
 if (!stralloc_cat(&dtline,&envrecip)) temp_nomem();
 for (i = 0;i < dtline.len;++i) if (dtline.s[i] == '\n') dtline.s[i] = '_';
 if (!stralloc_cats(&dtline,"\n")) temp_nomem();

 if (!stralloc_copy(&foo,&dtline)) temp_nomem();
 if (!stralloc_0(&foo)) temp_nomem();
 if (!env_put2("DTLINE",foo.s)) temp_nomem();

 if (flagdoit)
   bouncexf();

 if (!env_put2("SENDER",sender)) temp_nomem();

 if (!quote2(&foo,sender)) temp_nomem();
 if (!stralloc_copys(&rpline,"Return-Path: <")) temp_nomem();
 if (!stralloc_cat(&rpline,&foo)) temp_nomem();
 for (i = 0;i < rpline.len;++i) if (rpline.s[i] == '\n') rpline.s[i] = '_';
 if (!stralloc_cats(&rpline,">\n")) temp_nomem();

 if (!stralloc_copy(&foo,&rpline)) temp_nomem();
 if (!stralloc_0(&foo)) temp_nomem();
 if (!env_put2("RPLINE",foo.s)) temp_nomem();

 if (!stralloc_copys(&ufline,"From ")) temp_nomem();
 if (*sender)
  {
   int len; int i; char ch;

   len = str_len(sender);
   if (!stralloc_readyplus(&ufline,len)) temp_nomem();
   for (i = 0;i < len;++i)
    {
     ch = sender[i];
     if ((ch == ' ') || (ch == '\t') || (ch == '\n')) ch = '-';
     ufline.s[ufline.len + i] = ch;
    }
   ufline.len += len;
  }
 else
   if (!stralloc_cats(&ufline,"MAILER-DAEMON")) temp_nomem();
 if (!stralloc_cats(&ufline," ")) temp_nomem();
 starttime = now();
 if (!stralloc_cats(&ufline,myctime(starttime))) temp_nomem();

 if (!stralloc_copy(&foo,&ufline)) temp_nomem();
 if (!stralloc_0(&foo)) temp_nomem();
 if (!env_put2("UFLINE",foo.s)) temp_nomem();

 if (!stralloc_copys(&dashext,dash)) temp_nomem();
 if (!stralloc_cats(&dashext,ext)) temp_nomem();
 for (i = 0;i < dashext.len;++i)
   if (dashext.s[i] == '.')
     dashext.s[i] = ':';
 case_lowerb(dashext.s,dashext.len);

 extx = ext;
 if (!env_put2("EXT",extx)) temp_nomem();
 extx += str_chr(extx,'-'); if (*extx) ++extx;
 if (!env_put2("EXT2",extx)) temp_nomem();
 extx += str_chr(extx,'-'); if (*extx) ++extx;
 if (!env_put2("EXT3",extx)) temp_nomem();
 extx += str_chr(extx,'-'); if (*extx) ++extx;
 if (!env_put2("EXT4",extx)) temp_nomem();

 flagforwardonly = 0;
 fd = qmeopen(&flagforwardonly);
 if (fd == -1) if (*dash) bounce_ext();

 if (!stralloc_copys(&ueo,sender)) temp_nomem();
 if (str_diff(sender,""))
   if (str_diff(sender,"#@[]"))
     if (qmeox("-owner") == 0)
      {
       if (qmeox("-owner-default") == 0)
	{
         if (!stralloc_copys(&ueo,local)) temp_nomem();
         if (!stralloc_cats(&ueo,"-owner-@")) temp_nomem();
         if (!stralloc_cats(&ueo,host)) temp_nomem();
         if (!stralloc_cats(&ueo,"-@[]")) temp_nomem();
	}
       else
	{
         if (!stralloc_copys(&ueo,local)) temp_nomem();
         if (!stralloc_cats(&ueo,"-owner@")) temp_nomem();
         if (!stralloc_cats(&ueo,host)) temp_nomem();
	}
      }
 if (!stralloc_0(&ueo)) temp_nomem();
 if (!env_put2("NEWSENDER",ueo.s)) temp_nomem();

 if (!stralloc_ready(&cmds,0)) temp_nomem();
 cmds.len = 0;
 if (fd != -1)
   if (slurpclose(fd,&cmds,256) == -1) temp_nomem();

 if (!cmds.len)
  {
   if (!stralloc_copys(&cmds,aliasempty)) temp_nomem();
   flagforwardonly = 0;
  }
 if (!cmds.len || (cmds.s[cmds.len - 1] != '\n'))
   if (!stralloc_cats(&cmds,"\n")) temp_nomem();

 numforward = 0;
 i = 0;
 for (j = 0;j < cmds.len;++j)
   if (cmds.s[j] == '\n')
    {
     switch(cmds.s[i]) { case '#': case '.': case '/': case '|': break;
       default: ++numforward; }
     i = j + 1;
    }

 recips = (char **) alloc((numforward + 1) * sizeof(char *));
 if (!recips) temp_nomem();
 numforward = 0;

 flag99 = 0;

 i = 0;
 for (j = 0;j < cmds.len;++j)
   if (cmds.s[j] == '\n')
    {
     cmds.s[j] = 0;
     k = j;
     while ((k > i) && (cmds.s[k - 1] == ' ') || (cmds.s[k - 1] == '\t'))
       cmds.s[--k] = 0;
     switch(cmds.s[i])
      {
       case 0: /* k == i */
	 if (i) break;
	 temp_blankline();
       case '#':
         break;
       case '.':
       case '/':
	 ++count_file;
	 if (flagforwardonly) temp_fofile();
	 if (cmds.s[k - 1] == '/')
           if (flagdoit) maildir(cmds.s + i);
           else sayit("maildir ",cmds.s + i,k - i);
	 else
           if (flagdoit) mailfile(cmds.s + i);
           else sayit("mbox ",cmds.s + i,k - i);
         break;
       case '|':
	 ++count_program;
	 if (flagforwardonly) temp_foprog();
         if (flagdoit) mailprogram(cmds.s + i + 1);
         else sayit("program ",cmds.s + i + 1,k - i - 1);
         break;
       case '+':
	 if (str_equal(cmds.s + i + 1,"list"))
	   flagforwardonly = 1;
	 break;
       case '&':
         ++i;
       default:
	 ++count_forward;
         if (flagdoit) recips[numforward++] = cmds.s + i;
         else sayit("forward ",cmds.s + i,k - i);
         break;
      }
     i = j + 1;
     if (flag99) break;
    }

 if (numforward) if (flagdoit)
  {
   recips[numforward] = 0;
   mailforward(recips);
  }

 count_print();
 _exit(0);
}
