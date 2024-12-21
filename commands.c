#include "commands.h"

#include "substdio.h"
#include "stralloc.h"
#include "str.h"
#include "case.h"

void execute_command(char *line, size_t len, const struct command *cmds)
{
  size_t whitespace_pos, i;
  char *arg;
  /* trim newlines and carriage returns */
  while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n'))
    len--;

  line[len] = '\0';
  whitespace_pos = str_chr(line, ' ');
  arg = line + whitespace_pos;
  while (*arg == ' ')
    arg++;

  for (i = 0; cmds[i].text; i++) {
    if (!case_diffb(cmds[i].text, whitespace_pos, line))
      break;
  }

  cmds[i].fun(arg, cmds[i].data);
  if (cmds[i].flush) /* TODO does it belong here? */
    cmds[i].flush();
}
