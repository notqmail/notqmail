#include "substdio.h"
#include "subfd.h"
#include "readwrite.h"

char host[256];

int main(void)
{
 host[0] = 0; /* sigh */
 gethostname(host,sizeof(host));
 host[sizeof(host) - 1] = 0;
 substdio_puts(subfdoutsmall,host);
 substdio_puts(subfdoutsmall,"\n");
 substdio_flush(subfdoutsmall);
 return 0;
}
