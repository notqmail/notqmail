#include "cdb.h"

uint32 cdb_hash(const char *b, unsigned int len)
{
  uint32 h;

  h = 5381;
  while (len) {
   const unsigned char *buf = (const unsigned char *)b++;
    --len;
    h += (h << 5);
    h ^= (uint32) *buf++;
  }
  return h;
}
