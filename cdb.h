#ifndef CDB_H
#define CDB_H

#include "uint32.h"

extern uint32 cdb_hash();
extern uint32 cdb_unpack();

extern int cdb_bread();
extern int cdb_seek();

#endif
