#include "cdb.h"

uint32 cdb_hash(buf,len)
unsigned char *buf;
unsigned int len;
{
  uint32 h;

  h = 5381;
  while (len) {
    --len;
    h += (h << 5);
    h ^= (uint32) *buf++;
  }
  return h;
}
