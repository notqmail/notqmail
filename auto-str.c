#include <ctype.h>
#include "substdio.h"
#include "readwrite.h"
#include "exit.h"

char buf1[256];
substdio ss1 = SUBSTDIO_FDBUF(write,1,buf1,sizeof(buf1));

void puts(s)
char *s;
{
  if (substdio_puts(&ss1,s) == -1) _exit(111);
}

// check if a given character can be printed unquoted in a C string
// does not accept digits as they may be hardly visible between octal encoded chars
static int is_legible(unsigned char ch)
{
  if (isascii(ch))
    return 1;
  if (ch == '/' || ch == '_' || ch == '-' || ch == '.')
    return 1;
  return 0;
}

int main(int argc, char **argv)
{
  char *name;
  char *value;
  unsigned char ch;
  char octal[4];

  if (argc != 3) return 100;
  name = argv[1];
  if (!name) return 100;
  value = argv[2];
  if (!value) return 100;

  puts("char ");
  puts(name);
  puts("[] = \"\\\n");

  while ((ch = *value++)) {
    if (is_legible(ch)) {
      if (substdio_put(&ss1, &ch, 1) == -1)
        _exit(111);
    } else {
      puts("\\");
      octal[3] = 0;
      octal[2] = '0' + (ch & 7); ch >>= 3;
      octal[1] = '0' + (ch & 7); ch >>= 3;
      octal[0] = '0' + (ch & 7);
      puts(octal);
    }
  }

  puts("\\\n\";\n");
  if (substdio_flush(&ss1) == -1) return 111;
  return 0;
}
