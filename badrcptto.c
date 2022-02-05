#include <unistd.h>
#include "byte.h"
#include "constmap.h"
#include "control.h"
#include "env.h"
#include "fmt.h"
#include "str.h"
#include "stralloc.h"
#include "strerr.h"

extern void die_control();
extern void die_nomem();

static void _badrcptto_log_rejection(char *recipient)
{
  char smtpdpid[32];
  char *remoteip = env_get("TCPREMOTEIP");
  if (!remoteip) remoteip = "unknown";
  str_copy(smtpdpid + fmt_ulong(smtpdpid,getppid())," ");
  strerr_warn5("rcptcheck: badrcptto ",smtpdpid,remoteip," ",recipient,0);
}

static int _badrcptto_reject_exact_address(struct constmap map, stralloc address)
{
  return (1 && constmap(&map,address.s,address.len - 1));
}

static int _badrcptto_reject_whole_domain(struct constmap map, stralloc address)
{
  /* why not just comment out the domain in control/rcpthosts? */
  int j = byte_rchr(address.s,address.len,'@');
  return ((j < address.len) && (constmap(&map,address.s + j,address.len - j - 1)));
}

static int _badrcptto_reject_string(char *string)
{
  stralloc addr = {0};
  stralloc brt = {0};
  struct constmap mapbrt;
  int brtok = control_readfile(&brt,"control/badrcptto",0);
  if (brtok == -1) die_control();
  if (!brtok) return 0;
  if (!constmap_init(&mapbrt,brt.s,brt.len,0)) die_nomem();

  if (!stralloc_copys(&addr,string)) die_nomem();
  if (!stralloc_0(&addr)) die_nomem();

  if (_badrcptto_reject_exact_address(mapbrt,addr)) {
    _badrcptto_log_rejection(addr.s);
    return 1;
  }

  if (_badrcptto_reject_whole_domain(mapbrt,addr)) {
    _badrcptto_log_rejection(addr.s);
    return 1;
  }

  return 0;
}

int badrcptto_reject_recipient(char *recipient)
{
  if (env_get("RELAYCLIENT"))
    return 0;

  return _badrcptto_reject_string(recipient);
}
