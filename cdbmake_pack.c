#include "cdbmake.h"

void cdbmake_pack(buf,num)
unsigned char *buf;
uint32 num;
{
  *buf++ = num; num >>= 8;
  *buf++ = num; num >>= 8;
  *buf++ = num; num >>= 8;
  *buf = num;
}
