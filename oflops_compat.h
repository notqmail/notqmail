#ifndef OFLOPS_H
#define OFLOPS_H

static inline int check_ofl(unsigned long long val, unsigned int *res)
{
	if (val >> 32)
		return 1;
	*res = (unsigned int)val;
	return 0;
}

static inline int __builtin_add_overflow(unsigned int a, unsigned int b, unsigned int *res)
{
	unsigned long long val = a;
	val += b;
	return check_ofl(val, res);
}

static inline int __builtin_mul_overflow(unsigned int a, unsigned int b, unsigned int *res)
{
	unsigned long long val = a;
	val *= b;
	return check_ofl(val, res);
}

#endif /* OFLOPS_H */

