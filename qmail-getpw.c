#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "readwrite.h"
#include "substdio.h"
#include "subfd.h"
#include "error.h"
#include "exit.h"
#include "byte.h"
#include "str.h"
#include "case.h"
#include "fmt.h"
#include "auto_usera.h"
#include "auto_break.h"
#include "qlx.h"

#define GETPW_USERLEN 32

char *local;
struct passwd *pw;
char *dash;
char *extension;

int userext()
{
  char username[GETPW_USERLEN];
  struct stat st;

  extension = local + str_len(local);
  for (;;) {
    if (extension - local < sizeof(username))
      if (!*extension || (*extension == *auto_break)) {
	byte_copy(username,extension - local,local);
	username[extension - local] = 0;
	case_lowers(username);
	errno = 0;
	pw = getpwnam(username);
	if (errno == error_txtbsy) _exit(QLX_SYS);
	if (pw)
	  if (pw->pw_uid)
	    if (stat(pw->pw_dir,&st) == 0) {
	      if (st.st_uid == pw->pw_uid) {
		dash = "";
		if (*extension) { ++extension; dash = "-"; }
		return 1;
	      }
	    }
	    else
	      if (error_temp(errno)) _exit(QLX_NFS);
      }
    if (extension == local) return 0;
    --extension;
  }
}

char num[FMT_ULONG];

void main(argc,argv)
int argc;
char **argv;
{
  local = argv[1];
  if (!local) _exit(100);

  if (!userext()) {
    extension = local;
    dash = "-";
    pw = getpwnam(auto_usera);
  }

  if (!pw) _exit(QLX_NOALIAS);

  substdio_puts(subfdoutsmall,pw->pw_name);
  substdio_put(subfdoutsmall,"",1);
  substdio_put(subfdoutsmall,num,fmt_ulong(num,(long) pw->pw_uid));
  substdio_put(subfdoutsmall,"",1);
  substdio_put(subfdoutsmall,num,fmt_ulong(num,(long) pw->pw_gid));
  substdio_put(subfdoutsmall,"",1);
  substdio_puts(subfdoutsmall,pw->pw_dir);
  substdio_put(subfdoutsmall,"",1);
  substdio_puts(subfdoutsmall,dash);
  substdio_put(subfdoutsmall,"",1);
  substdio_puts(subfdoutsmall,extension);
  substdio_put(subfdoutsmall,"",1);
  substdio_flush(subfdoutsmall);

  _exit(0);
}
