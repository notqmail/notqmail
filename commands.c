#include "commands.h"
#include "substdio.h"
#include "stralloc.h"
#include "str.h"
#include "case.h"

static stralloc cmd = {0};

int commands(ss,c)
substdio *ss;
struct commands *c;
{
  int i;
  char *arg;

  for (;;) {
    if (!stralloc_copys(&cmd,"")) return -1;

    for (;;) {
      if (!stralloc_readyplus(&cmd,1)) return -1;
      i = substdio_get(ss,cmd.s + cmd.len,1);
      if (i != 1) return i;
      if (cmd.s[cmd.len] == '\n') break;
      ++cmd.len;
    }

    if (cmd.len > 0) if (cmd.s[cmd.len - 1] == '\r') --cmd.len;

    cmd.s[cmd.len] = 0;

    i = str_chr(cmd.s,' ');
    arg = cmd.s + i;
    while (*arg == ' ') ++arg;
    cmd.s[i] = 0;

    for (i = 0;c[i].text;++i) if (case_equals(c[i].text,cmd.s)) break;
    c[i].fun(arg);
    if (c[i].flush) c[i].flush();
  }
}
