#ifndef NORETURN_H
#define NORETURN_H

#if defined(__clang__) || defined(__GNUC__)
#define _noreturn_ __attribute__((noreturn))
#else
#define _noreturn_
#endif

#endif
