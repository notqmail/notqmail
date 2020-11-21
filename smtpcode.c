#include "smtpcode.h"

#include "substdio.h"
#include "stralloc.h"

#define HUGESMTPTEXT 5000

extern stralloc smtptext;
extern substdio smtpfrom;

static void get(unsigned char *uc)
{
  char *ch = (char *)uc;
  substdio_get(&smtpfrom,ch,1);
  if (*ch != '\r')
    if (smtptext.len < HUGESMTPTEXT)
     if (!stralloc_append(&smtptext,ch)) temp_nomem();
}

static int get_digit(unsigned char *uc)
{
  get(uc);
  if (*uc < '0' || *uc > '9')
    return 1;
  return 0;
}

unsigned long smtpcode()
{
  unsigned char ch;
  unsigned long code;
  int err = 0;

  if (!stralloc_copys(&smtptext,"")) temp_nomem();

  err += get_digit(&ch); code = ch - '0';
  /* valid SMTP codes are >= 200 */
  if (ch < '2')
    err++;
  err += get_digit(&ch); code = code * 10 + (ch - '0');
  err += get_digit(&ch); code = code * 10 + (ch - '0');

  for (;;) {
    get(&ch);
    if (ch != ' ' && ch != '-')
      err++;
    if (ch != '-') break;
    while (ch != '\n') get(&ch);
    err += get_digit(&ch);
    if (ch - '0' != code / 100)
      err++;
    err += get_digit(&ch);
    if (ch - '0' != (code / 10) % 10)
      err++;
    err += get_digit(&ch);
    if (ch - '0' != code % 10)
      err++;
  }
  while (ch != '\n') get(&ch);

  /* Make an invalid response code look like a permanent error (>500) to callers
   * without letting it be mistaken for a real 3-digit response code. */
  if (err)
    return 9999;
  return code;
}
