#ifndef QMAIL_QFILTER_H
#define QMAIL_QFILTER_H

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

#ifndef BUFSIZE
#define BUFSIZE 4096
#endif

#define QQ_DROP_MESSAGE 99

#define MESSAGE_IN 0
#define MESSAGE_OUT 1
#define ENVELOPE_IN 3
#define ENVELOPE_OUT 4
#define QMAILQUEUE_OVERRIDE 5

struct command
{
  char **argv;
  struct command *next;
};
typedef struct command command;

//for main()
command *parse_args_to_linked_list_of_filters(int,char *[]);
void prepare_to_run_filters(void);
void run_filters_in_sequence(const command *);
void setup_qqargs(int);
void exec_qq(void);

//for tests
#include <sys/types.h>
size_t parse_sender(const char *);
void qqf_debug_ulong(const char *,unsigned long);
void die(int);

#endif
