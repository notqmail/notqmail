#ifndef TOKEN822_H
#define TOKEN822_H

struct token822
 {
  int type;
  char *s;
  int slen;
 }
;

#include "gen_alloc.h"
GEN_ALLOC_typedef(token822_alloc,struct token822,t,len,a)

extern int token822_parse();
extern int token822_addrlist();
extern int token822_unquote();
extern int token822_unparse();
extern void token822_free();
extern void token822_reverse();
extern int token822_ready();
extern int token822_readyplus();
extern int token822_append();

#define TOKEN822_ATOM 1
#define TOKEN822_QUOTE 2
#define TOKEN822_LITERAL 3
#define TOKEN822_COMMENT 4
#define TOKEN822_LEFT 5
#define TOKEN822_RIGHT 6
#define TOKEN822_AT 7
#define TOKEN822_COMMA 8
#define TOKEN822_SEMI 9
#define TOKEN822_COLON 10
#define TOKEN822_DOT 11

#endif
