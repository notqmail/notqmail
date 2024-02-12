#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "control.h"
#include "error.h"
#include "stralloc.h"
#include "env.h"
#include "getDomainToken.h"

extern char **environ;

extern dtype    delivery;

int
main(int argc, char **argv)
{
	char           *filterargs;
	char          **ptr;
	static stralloc filterdefs = { 0 };

	if (argc != 2) {
		fprintf(stderr, "USAGE: testfilterargs domain\n");
		_exit(100);
	}
	env_clear();
	delivery = remote_delivery;
	chdir("/var/qmail");
	if (control_readfile(&filterdefs, "control/filterargs", 0) == -1) {
		fprintf(stderr, "unable to open filterargs: %s\n", strerror(errno));
		_exit(111);
	}
	filterargs = getDomainToken(argv[1], &filterdefs);
	if (filterargs)
		printf("command=%s\n", filterargs);
	printf("\nenvironment list\n");
	for (ptr = environ; *ptr; ptr++)
		printf("%s\n", *ptr);
	return 0;
}
