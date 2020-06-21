#ifndef CDB_H
#define CDB_H

#include "uint32.h"

extern uint32 cdb_hash(const char *b, unsigned int len);
extern uint32 cdb_unpack(const char *b);

extern int cdb_bread(int fd, char *buf, int len);
extern int cdb_seek(int fd, const char *key, unsigned int len, uint32 *dlen);

#endif
