/* util.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstdio>
#include <cstddef>
#include <cstring>
#include <cerrno>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <err.h>

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
		//fprintf(stderr, "receiving\n");
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

			warn("couldn't recv()");
		} else if (r == 0) {
			warnx("empty response");
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
			warnx("wut?");

		if (from) {
			char portstr[7];
			snprintf(portstr, sizeof portstr, ":%hu", port);
			Util::strNcat(from, portstr, fromsz);
		}

		return r;

	}
	warnx("timeout");

	return 0;
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

};
