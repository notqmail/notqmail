#ifndef ENV_H
#define ENV_H

extern int env_isinit;

extern int env_init();
extern int env_put();
extern int env_put2();
extern int env_unset();
extern /*@null@*/char *env_get();
extern char *env_pick();
extern void env_clear();
extern char *env_findeq();

extern char **environ;

#endif
