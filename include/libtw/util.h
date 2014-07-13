/* util.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_UTIL_H
#define LIBTW_UTIL_H 1

#include <cstdint>

namespace tw {

class Util {
public:
	static uint64_t tstamp();
	static void tconv(struct timeval *tv, uint64_t *ts, bool tv_to_ts);

	static ssize_t Recv(int sck, void *buf, size_t len, uint64_t to_us,
			int sleep_us = 10000, char *from = NULL,
			size_t fromsz = 0);
	static ssize_t Send(int sck, void *buf, size_t len, const char *destaddr);

	static void strNcat(char *dest, const char *src, size_t destsz);
	static void hexdump(const void *pAddressIn, long lSize,
			const char *name);
};

};

#endif /* LIBTW_UTIL_H */
