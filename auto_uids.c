#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/types.h>
#include "auto_uids.h"
#include "auto_usergroupnames.h"
#include "qlx.h"

struct group *getgrnam();
struct passwd *getpwnam();

static int ids[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

static int
name2uid(name)
char *name;
{
  struct passwd *pw;
  pw = getpwnam(name);
  if (!pw) _exit(QLX_NOALIAS);
  return (int)(pw->pw_uid);
}

static int
name2gid(name)
char *name;
{
  struct group *gr;
  gr = getgrnam(name);
  if (!gr) _exit(QLX_NOALIAS);
  return (int)(gr->gr_gid);
}

int
qmail_id_lookup(id)
int id;
{
  if (ids[id] >= 0) return ids[id];

  switch(id) {
  case ID_OWNER:   ids[id] = name2uid(auto_usero); break;
  case ID_ALIAS:   ids[id] = name2uid(auto_usera); break;
  case ID_DAEMON:  ids[id] = name2uid(auto_userd); break;
  case ID_LOG:     ids[id] = name2uid(auto_userl); break;
  case ID_PASSWD:  ids[id] = name2uid(auto_userp); break;
  case ID_QUEUE:   ids[id] = name2uid(auto_userq); break;
  case ID_REMOTE:  ids[id] = name2uid(auto_userr); break;
  case ID_SEND:    ids[id] = name2uid(auto_users); break;
  case ID_QMAIL:   ids[id] = name2gid(auto_groupq); break;
  case ID_NOFILES: ids[id] = name2gid(auto_groupn); break;
  default: _exit(QLX_NOALIAS);
  }
  return ids[id];
}
