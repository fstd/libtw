/* info.cpp - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <err.h>

extern "C" {
#include <libsrsbsns/addr.h>
}

#include <libtw/info.h>
#include <libtw/util.h>


/* sort in somewhere */
#define CHUNKSZ 100

namespace tw {

InfoComm::InfoComm()
: infomap_(),
  pg_(),
  tok_(0),
  to_(1000000)
{
}

void
InfoComm::SetServers(vector<string> const& srvs)
{
	infomap_.clear();

	for(vector<string>::const_iterator it = srvs.begin();
			it != srvs.end(); it++) {
		infomap_[*it].tsend_ = infomap_[*it].trecv_ = 0;
		infomap_[*it].on_ = false;
	}
}

int
InfoComm::Refresh()
{
	/*broadcast since we might add LAN support some day*/
	int sck = addr_bind_socket_dgram("0.0.0.0", 0, true, true);

	if (sck < 0) {
		warnx("couldn't make socket");
		return -1;
	}

	int suc = 0;

	vector<string> addrs;
	for(map<string, ServerInfo>::const_iterator it = infomap_.begin();
			it != infomap_.end(); it++)
		addrs.push_back(it->first);

	for(size_t i = 0; i < addrs.size(); i += CHUNKSZ) {
		size_t n = i+CHUNKSZ <= addrs.size()
				?  CHUNKSZ : addrs.size() - i;

		int r = RefreshChunk(sck, tok_, addrs.begin() + i, n);
		if (r < 0)
			warnx("failed to refresh chunk\n");
		else
			suc += r;
	}

	close(sck);

	return suc;
}

int
InfoComm::RefreshChunk(int sck, unsigned char tok,
		vector<string>::const_iterator start, size_t num)
{
	int suc = 0;
	unsigned char pkt[16];
	size_t sz = pg_.MkConnless_SB_GETINFO(pkt, sizeof pkt, tok);

	for(size_t i = 0; i < num; ((start++), (i++))) {
		struct sockaddr_storage sa;
		if (!addr_make_sockaddr(start->c_str(), (sockaddr*)&sa)) {
			warnx("couldn't make sockaddr for '%s'",
					start->c_str());
			continue;
		}

		//warnx("sending to '%s'", start->c_str());

		uint64_t tsend = Util::tstamp();
		errno = 0;
		ssize_t r = sendto(sck, pkt, sz, MSG_NOSIGNAL,
				(sockaddr*)&sa, sizeof sa);
		if (r == -1) {
			warn("couldn't send()");
			continue;
		}

		infomap_[*start].tsend_ = tsend;
		infomap_[*start].on_ = false;
	}

	for(;;) {
		char from[128] = {0};
		char buf[4096];
		ssize_t r = Util::Recv(sck, buf, sizeof buf, to_, 1000,
				from, sizeof from);

		if (r <= 0)
			break;

		warnx("received %zd bytes from '%s'", r, from);
		suc++;

	}
	return suc;
}


};
