#include "hassmtputf8.h"

#ifdef SMTPUTF8
#include "utf8read.h"
#include "stralloc.h"
#include "case.h"
#include "substdio.h"
#include "subfd.h"

static stralloc receivedline = { 0 };
stralloc        header = { 0 };
extern void temp_nomem();
extern void temp_read();

int
containsutf8(unsigned char *p, int l)
{
	int             i = 0;

	while (i < l)
		if (p[i++] > 127)
			return 1;
	return 0;
}

int
utf8read()
{
  int             r, i, received = 0;
  char            ch;

  if (flagutf8)
    return 1;
  /*- initialize headers so that we don't create if called more than once */
  header.len = receivedline.len = 0;
  for (;;) {
    r = substdio_get(subfdin, &ch, 1);
    if (r == 0) break;
    if (r == -1) temp_read();

    if (ch == '\n') {
      if (!stralloc_append(&header, "\r")) temp_nomem(); /* received.c does not add '\r' */
      if (!stralloc_append(&header, "\n")) temp_nomem();
      if (case_starts(receivedline.s, "Date:")) return 0;  /* header to quit asap */
      if (case_starts(receivedline.s, "Received: from")) received++;  /* found Received header */
      if (received) {
        for (i = 0; i < receivedline.len; i++)
          if (*(receivedline.s + i) != ' ' && *(receivedline.s + i) != '\t')
            break;
        if (case_starts(receivedline.s + i, "by ")) {
          for (i += 3; i < receivedline.len; ++i)
            if (*(receivedline.s + i) == ' ' || *(receivedline.s + i) == '\t')
              if (case_starts(receivedline.s + i + 1, "with UTF8")) {
                flagutf8 = 1;return 1;
              }
          return 0;
        }
        for (i = 0; i < receivedline.len; i++) {
          if (case_starts(receivedline.s + i, "by ")) {
            for (i += 3; i < receivedline.len; ++i)
              if (*(receivedline.s + i) == ' ' || *(receivedline.s + i) == '\t')
                if (case_starts(receivedline.s + i + 1, "with UTF8")) {
                  flagutf8 = 1;return 1;
                }
            return 0;
          }
        }
      }
      if (!stralloc_copys(&receivedline, "")) temp_nomem();
    } else {
      if (!stralloc_append(&header, &ch)) temp_nomem();
      if (!stralloc_catb(&receivedline, &ch, 1)) temp_nomem();
    }
  }
  return 0;
}
#endif
