#include <sys/types.h>
#include <fcntl.h>
#include "open.h"

int open_excl(fn) char *fn;
{ return open(fn,O_WRONLY | O_EXCL | O_CREAT,0644); }
