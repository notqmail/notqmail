#include "cdb.h"

uint32 cdb_unpack(const char *b)
{
  uint32 num;
  const unsigned char *buf = (const unsigned char*)b;
  num = buf[3]; num <<= 8;
  num += buf[2]; num <<= 8;
  num += buf[1]; num <<= 8;
  num += buf[0];
  return num;
}
