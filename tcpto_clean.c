#include "tcpto.h"
#include "open.h"
#include "substdio.h"
#include "readwrite.h"

char tcpto_cleanbuf[1024];

void tcpto_clean() /* running from queue/mess */
{
 int fd;
 int i;
 substdio ss;

 fd = open_write("../lock/tcpto");
 if (fd == -1) return;
 substdio_fdbuf(&ss,write,fd,tcpto_cleanbuf,sizeof(tcpto_cleanbuf));
 for (i = 0;i < sizeof(tcpto_cleanbuf);++i) substdio_put(&ss,"",1);
 substdio_flush(&ss); /* if it fails, bummer */
 close(fd);
}
