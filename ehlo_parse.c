#include "ehlo_parse.h"

#include "case.h"
#include "str.h"

unsigned int ehlo_parse(const stralloc *smtptext, const struct smtpext *callbacks, unsigned int count)
{
  /* if this is a one line answer there will be no extensions */
  if (smtptext->len < 4 || smtptext->s[4] == ' ')
    return 0;

  size_t search = 0;
  unsigned int extensions = 0;
  const unsigned int maxmask = (1 << count) - 1;

  /* go through all lines of the multi line answer until we found all
     known extensions or we reach the last line */
  do {
    unsigned int i;

    /* set search to the index of the next extension in the answer:
       it's always 5 characters after the '\n' (the other 4 are
       normally "250-") */
    search += 5 + str_chr(smtptext->s + search, '\n');

    for (i = 0; i < count; i++) {
      const size_t elen = strlen(callbacks[i].name);
      if (!case_diffb(smtptext->s + search, elen, callbacks[i].name)) {
        if (smtptext->s[search + elen] == '\n' ||
            smtptext->s[search + elen] == ' ') {
          if (callbacks[i].callback) {
            if (callbacks[i].callback(smtptext->s + search, str_chr(smtptext->s + search, '\n')))
              extensions |= (1 << i);
          } else {
            extensions |= (1 << i);
          }
          break;
        }
      }
    }
    /* all known extensions found, no need to search any longer */
    if (extensions == maxmask)
      break;
  } while (smtptext->s[search - 1] == '-');

  return extensions;
}
