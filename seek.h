#ifndef SEEK_H
#define SEEK_H

typedef unsigned long seek_pos;

extern seek_pos seek_cur();

extern int seek_set();
extern int seek_end();

extern int seek_trunc();

#define seek_begin(fd) (seek_set((fd),(seek_pos) 0))

#endif
