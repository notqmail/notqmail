#ifndef CDBMAKE_H
#define CDBMAKE_H

#include "uint32.h"

#define CDBMAKE_HPLIST 1000

struct cdbmake_hp { uint32 h; uint32 p; } ;

struct cdbmake_hplist {
  struct cdbmake_hp hp[CDBMAKE_HPLIST];
  struct cdbmake_hplist *next;
  int num;
} ;

struct cdbmake {
  char final[2048];
  uint32 count[256];
  uint32 start[256];
  struct cdbmake_hplist *head;
  struct cdbmake_hp *split; /* includes space for hash */
  struct cdbmake_hp *hash;
  uint32 numentries;
} ;

extern void cdbmake_pack();
#define CDBMAKE_HASHSTART ((uint32) 5381)
extern uint32 cdbmake_hashadd();

extern void cdbmake_init();
extern int cdbmake_add();
extern int cdbmake_split();
extern uint32 cdbmake_throw();

#endif
