#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

// XXX this entire file is from Bruce Guenter's GPL'd syncdir, nearly verbatim!

static int fdirsync(const char* filename, unsigned length)
{
  char dirname[length+1];
  /* This could also be:
   * char* dirname = alloca(length+1); */
  int dirfd;
  int retval;
  memcpy(dirname, filename, length);
  dirname[length] = 0;
  if((dirfd = open(dirname,O_RDONLY,0)) == -1)
    return -1;
  retval = (fsync(dirfd) == -1 && errno == EIO) ? -1 : 0;
  close(dirfd);
  return retval;
}

int fdirsyncfn(const char *filename)
{
   const char *slash = filename+strlen(filename)-1;

   /* Skip over trailing slashes, which would be ignored by some
    * operations */
   while(slash > filename && *slash == '/')
     --slash;

   /* Skip back to the next slash */
   while(slash > filename && *slash != '/')
     --slash;

   /* slash now either points to a '/' character, or no slash was found */
   if(*slash == '/')
      return fdirsync(filename,
                      (slash == filename) ? 1 : slash-filename);
   else
     return fdirsync(".", 1);
}
