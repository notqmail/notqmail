#include "cdb.h"

uint32 cdb_unpack(buf)
unsigned char *buf;
{
  uint32 num;
  num = buf[3]; num <<= 8;
  num += buf[2]; num <<= 8;
  num += buf[1]; num <<= 8;
  num += buf[0];
  return num;
}
