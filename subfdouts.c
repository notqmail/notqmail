#include "readwrite.h"
#include "substdio.h"
#include "subfd.h"

char subfd_outbufsmall[256];
static substdio it = SUBSTDIO_FDBUF(write,1,subfd_outbufsmall,256);
substdio *subfdoutsmall = &it;
