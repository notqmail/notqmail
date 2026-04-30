#ifndef LEAPSECS_H
#define LEAPSECS_H

#include "tai.h"

extern int leapsecs_init();

extern int leapsecs_sub(struct tai *);

#endif
