#include "qmail-qfilter.h"

#include <stdlib.h>

void die(int exitcode)
{
  qqf_debug_ulong("exit",exitcode);
  exit(exitcode);
}

int main(int argc, char* argv[])
{
  const command* filters = parse_args_to_linked_list_of_filters(argc-1, argv+1);
  prepare_to_run_filters();
  run_filters_in_sequence(filters);
  setup_qqargs(QMAILQUEUE_OVERRIDE);
  exec_qq();
}
