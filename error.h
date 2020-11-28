#ifndef ERROR_H
#define ERROR_H

#include <errno.h>

#define error_intr EINTR
#define error_nomem ENOMEM
#define error_noent ENOENT
#define error_txtbsy ETXTBSY
#define error_io EIO
#define error_exist EEXIST
#define error_timeout ETIMEDOUT
#define error_inprogress EINPROGRESS
#define error_wouldblock EWOULDBLOCK
#define error_again EAGAIN
#define error_pipe EPIPE
#define error_perm EPERM
#define error_acces EACCES

extern char *error_str();
extern int error_temp();

#endif
