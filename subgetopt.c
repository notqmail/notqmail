/* subgetopt.c, subgetopt.h: (yet another) improved getopt clone, inner layer
D. J. Bernstein, djb@pobox.com.
No dependencies.
No system requirements.
19970228: Cleanups.
931129: Adapted from getopt.c.
No known patent problems.

Documentation in subgetopt.3.
*/

#define SUBGETOPTNOSHORT
#include "subgetopt.h"

#define sgopt subgetopt
#define optind subgetoptind
#define optpos subgetoptpos
#define optarg subgetoptarg
#define optproblem subgetoptproblem
#define optdone subgetoptdone

int optind = 1;
int optpos = 0;
char *optarg = 0;
int optproblem = 0;
int optdone = SUBGETOPTDONE;

int sgopt(argc,argv,opts)
int argc;
char **argv;
char *opts;
{
  int c;
  char *s;

  optarg = 0;
  if (!argv || (optind >= argc) || !argv[optind]) return optdone;
  if (optpos && !argv[optind][optpos]) {
    ++optind;
    optpos = 0;
    if ((optind >= argc) || !argv[optind]) return optdone;
  }
  if (!optpos) {
    if (argv[optind][0] != '-') return optdone;
    ++optpos;
    c = argv[optind][1];
    if ((c == '-') || (c == 0)) {
      if (c) ++optind;
      optpos = 0;
      return optdone;
    }
    /* otherwise c is reassigned below */
  }
  c = argv[optind][optpos];
  ++optpos;
  s = opts;
  while (*s) {
    if (c == *s) {
      if (s[1] == ':') {
        optarg = argv[optind] + optpos;
        ++optind;
        optpos = 0;
        if (!*optarg) {
          optarg = argv[optind];
          if ((optind >= argc) || !optarg) { /* argument past end */
            optproblem = c;
            return '?';
          }
	  ++optind;
        }
      }
      return c;
    }
    ++s;
    if (*s == ':') ++s;
  }
  optproblem = c;
  return '?';
}
