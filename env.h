#ifndef ENV_H
#define ENV_H

extern int env_isinit;

extern int env_init();
extern int env_put(const char *s);
extern int env_put2(const char *s, const char *t);
extern int env_unset(const char *s);
extern /*@null@*/char *env_get(const char *s);
extern char *env_pick();
extern void env_clear();
extern const char *env_findeq(const char *s);

extern char **environ;

#endif
