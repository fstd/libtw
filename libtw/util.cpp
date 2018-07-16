/* util.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstdio>
#include <cstddef>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <err.h>

extern "C" {
#include <libsrsbsns/addr.h>
}

#include "debug.h"

#include <libtw/util.h>

namespace tw {

uint64_t
Util::tstamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((uint64_t)tv.tv_sec*1000000)+tv.tv_usec;
}

ssize_t
Util::Recv(int sck, void *buf, size_t len, uint64_t to_us, int sleep_us,
		char *from, size_t fromsz)
{
	sockaddr_storage sa;
	memset(&sa, 0, sizeof sa);
	uint64_t tend = Util::tstamp() + to_us;
	while(Util::tstamp() < tend) {
		errno = 0;
		socklen_t alen = sizeof sa;
		ssize_t r = recvfrom(sck, buf, len, MSG_DONTWAIT,
				(sockaddr*)&sa, &alen);
		if (r == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN
					|| errno == EINTR) {
				usleep(sleep_us);
				continue;
			}

			W("couldn't recv()");
		} else if (r == 0) {
			WX("empty response");
			usleep(sleep_us);
			continue;
		}

		unsigned short port = 0;

		if (from && sa.ss_family == AF_INET6) {
			inet_ntop(AF_INET6,
					&((sockaddr_in6*)&sa)->sin6_addr,
					from+1, fromsz-1);
			from[0] = '[';
			from[strlen(from)+1] = '\0'; //XXX
			from[strlen(from)] = ']';
			port = ntohs(((sockaddr_in6*)&sa)->sin6_port);
		} else if (from && sa.ss_family == AF_INET) {
			inet_ntop(AF_INET, &((sockaddr_in*)&sa)->sin_addr,
					from, fromsz);
			port = ntohs(((sockaddr_in*)&sa)->sin_port);
		} else if (from)
			WX("wut?");

		if (from) {
			char portstr[7];
			snprintf(portstr, sizeof portstr, ":%hu", port);
			Util::strNcat(from, portstr, fromsz);
		}

		return r;

	}

	return 0;
}

ssize_t
Util::Send(int sck, void *buf, size_t len, const char *destaddr)
{
		struct sockaddr_storage sa;
		size_t saz = sizeof sa;
		if (destaddr && !addr_make_sockaddr(destaddr, (sockaddr*)&sa, &saz)) {
				WX("couldn't make sockaddr for '%s'", destaddr);
				return -1;
		}

		errno = 0;
		ssize_t r = sendto(sck, buf, len, MSG_NOSIGNAL,
						destaddr ? (sockaddr*)&sa : NULL,
						destaddr ? sizeof sa : 0);

		if (r == -1)
			W("couldn't send()");

		if (r < (ssize_t)len)
			WX("short send (%zd/%zu)", r, len);

		return r;
}

void
Util::strNcat(char *dest, const char *src, size_t destsz)
{
	size_t len = strlen(dest);
	if (len + 1 >= destsz)
		return;

	size_t rem = destsz - (len + 1);

	char *ptr = dest + len;
	while(rem-- && *src) {
		*ptr++ = *src++;
	}
	*ptr = '\0';
}

/*not-quite-ravomavain's h4xdump*/
void
Util::hexdump(const void *pAddressIn, long lSize, const char *name)
{
	char szBuf[100];
	long lIndent = 1;
	long lOutLen, lIndex, lIndex2, lOutLen2;
	long lRelPos;
	struct { char *pData; unsigned long lSize; } buf;
	unsigned char *pTmp,ucTmp;
	unsigned char *pAddress = (unsigned char *)pAddressIn;

	buf.pData   = (char *)pAddress;
	buf.lSize   = lSize;
	fprintf(stderr, "hexdump '%s'\n", name);

	while (buf.lSize > 0)
	{
		pTmp     = (unsigned char *)buf.pData;
		lOutLen  = (int)buf.lSize;
		if (lOutLen > 16)
			lOutLen = 16;

		/* create a 64-character formatted output line: */
		sprintf(szBuf, " |                            "
				"                      "
				"    %08lX", (long unsigned int)(pTmp-pAddress));
		lOutLen2 = lOutLen;

		for(lIndex = 1+lIndent, lIndex2 = 53-15+lIndent, lRelPos = 0;
				lOutLen2;
				lOutLen2--, lIndex += 2, lIndex2++
		   )
		{
			ucTmp = *pTmp++;

			sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
			if(!isprint(ucTmp))  ucTmp = '.'; /* nonprintable char */
			szBuf[lIndex2] = ucTmp;

			if (!(++lRelPos & 3))     /* extra blank after 4 bytes */
			{  lIndex++; szBuf[lIndex+2] = ' '; }
		}

		if (!(lRelPos & 3)) lIndex--;

		szBuf[lIndex  ]   = '|';
		szBuf[lIndex+1]   = ' ';

		fprintf(stderr, "%s\n", szBuf);

		buf.pData   += lOutLen;
		buf.lSize   -= lOutLen;
	}
	fprintf(stderr, "end of hexdump '%s'\n", name);
}

};
