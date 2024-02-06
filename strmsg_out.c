#include "substdio.h"
#include "subfd.h"

void
strmsg_out(char *x1, char *x2, char *x3, char *x4, char *x5, char *x6,
		char *x7, char *x8, char *x9, char *x10, char *x11, char *x12,
		char *x13, char *x14, char *x15, char *x16)
{
	if (x1)
		substdio_puts(subfdout, x1);
	if (x2)
		substdio_puts(subfdout, x2);
	if (x3)
		substdio_puts(subfdout, x3);
	if (x4)
		substdio_puts(subfdout, x4);
	if (x5)
		substdio_puts(subfdout, x5);
	if (x6)
		substdio_puts(subfdout, x6);
	if (x7)
		substdio_puts(subfdout, x7);
	if (x8)
		substdio_puts(subfdout, x8);
	if (x9)
		substdio_puts(subfdout, x9);
	if (x10)
		substdio_puts(subfdout, x10);
	if (x11)
		substdio_puts(subfdout, x11);
	if (x12)
		substdio_puts(subfdout, x12);
	if (x13)
		substdio_puts(subfdout, x13);
	if (x14)
		substdio_puts(subfdout, x14);
	if (x15)
		substdio_puts(subfdout, x15);
	substdio_flush(subfdout);
}
