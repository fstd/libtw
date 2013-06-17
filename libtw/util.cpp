/* util.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstdio>
#include <cstddef>
#include <cerrno>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
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
Util::Recv(int sck, void *buf, size_t len, uint64_t to_us)
{
	uint64_t tend = Util::tstamp() + to_us;
	while(Util::tstamp() < tend) {
		//fprintf(stderr, "receiving\n");
		errno = 0;
		ssize_t r = recv(sck, buf, len, MSG_DONTWAIT);
		if (r == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN
					|| errno == EINTR) {
				usleep(10000);
				continue;
			}

			warn("couldn't recv()");
		} else if (r == 0) {
			warnx("empty response");
			usleep(10000);
			continue;
		}

		return r;

	}
	warnx("timeout");

	return 0;
}

};
