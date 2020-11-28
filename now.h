#ifndef NOW_H
#define NOW_H

#include <stddef.h>
#include <time.h>

static inline time_t now() { return time(NULL); }

#endif
