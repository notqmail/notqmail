#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
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

// XXX static?
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

// XXX well, it's also borrowed from syncdir

int fsync_after_open_or_bust(const char *fn, const int fd)
{
  if (fd == -1)
    return fd;

  if (fdirsyncfn(fn) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}

int syncdir_link(const char *oldpath, const char *newpath)
{
  if (link(oldpath, newpath) == -1)
    return -1;

  return fdirsyncfn(newpath);
}

int syncdir_unlink(const char *path)
{
  if (unlink(path) == -1)
    return -1;

  return fdirsyncfn(path);
}

int syncdir_rename(const char *oldpath, const char *newpath)
{
  if (rename(oldpath,newpath) == -1)
    return -1;

  if (fdirsyncfn(newpath) == -1)
    return -1;

  return fdirsyncfn(oldpath);
}
