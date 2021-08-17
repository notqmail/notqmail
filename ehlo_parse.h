#ifndef EHLO_PARSE_H
#define EHLO_PARSE_H

#include "stralloc.h"

#include <sys/types.h>
#include <stdint.h>

/**
 * Callbacks for EHLO response parsing
 */
struct smtpext {
  const char *name; /**< name of the EHLO string */
  /**
   * callback in case a space character follows name in the EHLO response
   * The function is given the current extension line without the leading 250-
   * and the length of the remainder, not including the trailing newline. This
   * includes the name part of the line so one could reuse the same callback
   * for multiple extensions.
   *
   * The callback shall return 0 if the line was ignored, or 1 if it was
   * accepted.
   *
   * In case the callback is NULL the extension is automatically accepted if
   * the name is matched and either followed by a space or newline.
   */
  int (*callback)(const char *ext, size_t extlen);
};

/**
 * @brief parse the EHLO replies
 * @param smtptext the reply to parse
 * @param callbacks the list of callbacks
 * @param count number of entries in callbacks
 * @return mask of the matched callbacks
 *
 * Will parse all callbacks until either all lines are processed or all
 * callbacks have been matched.
 *
 * The return value will have the bit positions set according to the
 * entries in the callbacks param.
 */
unsigned int ehlo_parse(const stralloc *smtptext, const struct smtpext *callbacks, unsigned int count);

/*** SIZE extension */
extern int64_t remotesize; /**< SIZE supported by the remote server, -1 if extension not announced */
int ehlo_size(const char *ext, size_t extlen);

#endif
