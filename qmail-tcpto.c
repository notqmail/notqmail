/* XXX: this program knows quite a bit about tcpto's internals */

#include "substdio.h"
#include "subfd.h"
#include "auto_qmail.h"
#include "fmt.h"
#include "ip.h"
#include "lock.h"
#include "error.h"
#include "exit.h"
#include "datetime.h"
#include "now.h"

void die(n) int n; { substdio_flush(subfdout); _exit(n); }

void warn(s) char *s;
{
 char *x;
 x = error_str(errno);
 substdio_puts(subfdout,s);
 substdio_puts(subfdout,": ");
 substdio_puts(subfdout,x);
 substdio_puts(subfdout,"\n");
}

void die_chdir() { warn("fatal: unable to chdir"); die(111); }
void die_open() { warn("fatal: unable to open tcpto"); die(111); }
void die_lock() { warn("fatal: unable to lock tcpto"); die(111); }
void die_read() { warn("fatal: unable to read tcpto"); die(111); }

char tcpto_buf[1024];

char tmp[FMT_ULONG + IPFMT];

void main()
{
 int fdlock;
 int fd;
 int r;
 int i;
 char *record;
 struct ip_address ip;
 datetime_sec when;
 datetime_sec start;

 if (chdir(auto_qmail) == -1) die_chdir();
 if (chdir("queue/lock") == -1) die_chdir();

 fdlock = open_write("tcpto");
 if (fdlock == -1) die_open();
 fd = open_read("tcpto");
 if (fd == -1) die_open();
 if (lock_ex(fdlock) == -1) die_lock();
 r = read(fd,tcpto_buf,sizeof(tcpto_buf));
 close(fd);
 close(fdlock);

 if (r == -1) die_read();
 r >>= 4;

 start = now();

 record = tcpto_buf;
 for (i = 0;i < r;++i)
  {
   if (record[4] >= 1)
    {
     byte_copy(&ip,4,record);
     when = (unsigned long) (unsigned char) record[11];
     when = (when << 8) + (unsigned long) (unsigned char) record[10];
     when = (when << 8) + (unsigned long) (unsigned char) record[9];
     when = (when << 8) + (unsigned long) (unsigned char) record[8];

     substdio_put(subfdout,tmp,ip_fmt(tmp,&ip));
     substdio_puts(subfdout," timed out ");
     substdio_put(subfdout,tmp,fmt_ulong(tmp,(unsigned long) (start - when)));
     substdio_puts(subfdout," seconds ago; # recent timeouts: ");
     substdio_put(subfdout,tmp,fmt_ulong(tmp,(unsigned long) (unsigned char) record[4]));
     substdio_puts(subfdout,"\n");
    }
   record += 16;
  }

 die(0);
}
