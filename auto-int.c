#include "substdio.h"
#include "readwrite.h"
#include "exit.h"
#include "scan.h"
#include "fmt.h"

char buf1[256];
substdio ss1 = SUBSTDIO_FDBUF(write,1,buf1,sizeof(buf1));

void put_str(const char *s)
{
  if (substdio_puts(&ss1,s) == -1) _exit(111);
}

int main(int argc, char **argv)
{
  char *name;
  char *value;
  unsigned long num;
  char strnum[FMT_ULONG];

  if (argc != 3) return 100;
  name = argv[1];
  if (!name) return 100;
  value = argv[2];
  if (!value) return 100;

  scan_ulong(value,&num);
  strnum[fmt_ulong(strnum,num)] = 0;

  put_str("int ");
  put_str(name);
  put_str(" = ");
  put_str(strnum);
  put_str(";\n");
  if (substdio_flush(&ss1) == -1) return 111;
  return 0;
}
