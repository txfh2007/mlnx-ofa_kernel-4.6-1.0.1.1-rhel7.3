/*
 * Convert integer string representation to an integer.
 * If an integer doesn't fit into specified type, -E is returned.
 *
 * Integer starts with optional sign.
 * kstrtou*() functions do not accept sign "-".
 *
 * Radix 0 means autodetection: leading "0x" implies radix 16,
 * leading "0" implies radix 8, otherwise radix is 10.
 * Autodetection hints work after optional sign, but not before.
 *
 * If -E is returned, result is not touched.
 */
#include <linux/kernel.h>

#ifndef CONFIG_COMPAT_IS_KSTRTOX
/* 
 * kstrto* was included in kernel 2.6.38.4 and causes conflicts with the
 * version included in compat-wireless. We use strict_strtol to check if
 * kstrto* is already available.
 */
#ifndef strict_strtoll

#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/types.h>

static inline char _tolower(const char c)
{
	return c | 0x20;
}

static int _kstrtoull(const char *s, unsigned int base, unsigned long long *res)
{
	unsigned long long acc;
	int ok;

	if (base == 0) {
		if (s[0] == '0') {
			if (_tolower(s[1]) == 'x' && isxdigit(s[2]))
				base = 16;
			else
				base = 8;
		} else
			base = 10;
	}
	if (base == 16 && s[0] == '0' && _tolower(s[1]) == 'x')
		s += 2;

	acc = 0;
	ok = 0;
	while (*s) {
		unsigned int val;

		if ('0' <= *s && *s <= '9')
			val = *s - '0';
		else if ('a' <= _tolower(*s) && _tolower(*s) <= 'f')
			val = _tolower(*s) - 'a' + 10;
		else if (*s == '\n') {
			if (*(s + 1) == '\0')
				break;
			else
				return -EINVAL;
		} else
			return -EINVAL;

		if (val >= base)
			return -EINVAL;
		if (acc > div_u64(ULLONG_MAX - val, base))
			return -ERANGE;
		acc = acc * base + val;
		ok = 1;

		s++;
	}
	if (!ok)
		return -EINVAL;
	*res = acc;
	return 0;
}

#define kstrtoull LINUX_BACKPORT(kstrtoull)
int kstrtoull(const char *s, unsigned int base, unsigned long long *res)
{
	if (s[0] == '+')
		s++;
	return _kstrtoull(s, base, res);
}
EXPORT_SYMBOL_GPL(kstrtoull);

#define kstrtoll LINUX_BACKPORT(kstrtoll)
int kstrtoll(const char *s, unsigned int base, long long *res)
{
	unsigned long long tmp;
	int rv;

	if (s[0] == '-') {
		rv = _kstrtoull(s + 1, base, &tmp);
		if (rv < 0)
			return rv;
		if ((long long)(-tmp) >= 0)
			return -ERANGE;
		*res = -tmp;
	} else {
		rv = kstrtoull(s, base, &tmp);
		if (rv < 0)
			return rv;
		if ((long long)tmp < 0)
			return -ERANGE;
		*res = tmp;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(kstrtoll);

/* Internal, do not use. */
#define _kstrtoul LINUX_BACKPORT(_kstrtoul)
int _kstrtoul(const char *s, unsigned int base, unsigned long *res)
{
	unsigned long long tmp;
	int rv;

	rv = kstrtoull(s, base, &tmp);
	if (rv < 0)
		return rv;
	if (tmp != (unsigned long long)(unsigned long)tmp)
		return -ERANGE;
	*res = tmp;
	return 0;
}
EXPORT_SYMBOL_GPL(_kstrtoul);

/* Internal, do not use. */
#define _kstrtol LINUX_BACKPORT(_kstrtol)
int _kstrtol(const char *s, unsigned int base, long *res)
{
	long long tmp;
	int rv;

	rv = kstrtoll(s, base, &tmp);
	if (rv < 0)
		return rv;
	if (tmp != (long long)(long)tmp)
		return -ERANGE;
	*res = tmp;
	return 0;
}
EXPORT_SYMBOL_GPL(_kstrtol);

#define kstrtouint LINUX_BACKPORT(kstrtouint)
int kstrtouint(const char *s, unsigned int base, unsigned int *res)
{
	unsigned long long tmp;
	int rv;

	rv = kstrtoull(s, base, &tmp);
	if (rv < 0)
		return rv;
	if (tmp != (unsigned long long)(unsigned int)tmp)
		return -ERANGE;
	*res = tmp;
	return 0;
}
EXPORT_SYMBOL_GPL(kstrtouint);

#define kstrtoint LINUX_BACKPORT(kstrtoint)
int kstrtoint(const char *s, unsigned int base, int *res)
{
	long long tmp;
	int rv;

	rv = kstrtoll(s, base, &tmp);
	if (rv < 0)
		return rv;
	if (tmp != (long long)(int)tmp)
		return -ERANGE;
	*res = tmp;
	return 0;
}
EXPORT_SYMBOL_GPL(kstrtoint);

#define kstrtou16 LINUX_BACKPORT(kstrtou16)
int kstrtou16(const char *s, unsigned int base, u16 *res)
{
	unsigned long long tmp;
	int rv;

	rv = kstrtoull(s, base, &tmp);
	if (rv < 0)
		return rv;
	if (tmp != (unsigned long long)(u16)tmp)
		return -ERANGE;
	*res = tmp;
	return 0;
}
EXPORT_SYMBOL_GPL(kstrtou16);

#define kstrtos16 LINUX_BACKPORT(kstrtos16)
int kstrtos16(const char *s, unsigned int base, s16 *res)
{
	long long tmp;
	int rv;

	rv = kstrtoll(s, base, &tmp);
	if (rv < 0)
		return rv;
	if (tmp != (long long)(s16)tmp)
		return -ERANGE;
	*res = tmp;
	return 0;
}
EXPORT_SYMBOL_GPL(kstrtos16);

#define kstrtou8 LINUX_BACKPORT(kstrtou8)
int kstrtou8(const char *s, unsigned int base, u8 *res)
{
	unsigned long long tmp;
	int rv;

	rv = kstrtoull(s, base, &tmp);
	if (rv < 0)
		return rv;
	if (tmp != (unsigned long long)(u8)tmp)
		return -ERANGE;
	*res = tmp;
	return 0;
}
EXPORT_SYMBOL_GPL(kstrtou8);

#define kstrtos8 LINUX_BACKPORT(kstrtos8)
int kstrtos8(const char *s, unsigned int base, s8 *res)
{
	long long tmp;
	int rv;

	rv = kstrtoll(s, base, &tmp);
	if (rv < 0)
		return rv;
	if (tmp != (long long)(s8)tmp)
		return -ERANGE;
	*res = tmp;
	return 0;
}
EXPORT_SYMBOL_GPL(kstrtos8);
#endif /* #ifndef strict_strtol */
#endif /* #ifndef CONFIG_COMPAT_IS_KSTRTOX */
