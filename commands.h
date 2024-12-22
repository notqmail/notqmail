#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

struct command {
  char *text;
  void (*fun)(char *arg, void *data);
  void (*flush)(void);
  void *data;
} ;

extern void execute_command(char *line, size_t len, const struct command *cmds);

#endif
