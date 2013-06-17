/* master.cpp - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <err.h>

#include <unistd.h>

extern "C" {
#include <libsrsbsns/addr.h>
}

#include <libtw/util.h>
#include <libtw/master.h>

/*sort this in somewhere*/
#define FUZZ 40 /*wow...*/

namespace tw {

MasterComm::MasterComm()
: msrvs_(),
  numtries_(2),
  to_(500000),
  pg_()
{
}

void
MasterComm::SetMasters(vector<string> const& masters)
{
	for(set<pair<char*, unsigned short> >::const_iterator
			it = msrvs_.begin(); it != msrvs_.end(); it++)
		free(it->first);
	
	msrvs_.clear();

	for(vector<string>::const_iterator it = masters.begin();
			it != masters.end(); it++) {
		char host[256];
		unsigned short port;
		addr_parse_hostspec(host, sizeof host, &port, it->c_str());
		msrvs_.insert(pair<char*, unsigned short>(
				strdup(host), port ? port : 8300));
	}
}

void
MasterComm::GetList(vector<string> & result)
{
	if (msrvs_.size() == 0)
		return;
	
	int sck = -1;

	for(set<pair<char*, unsigned short> >::const_iterator
			it = msrvs_.begin(); it != msrvs_.end(); it++) {

		sck = addr_connect_socket_dgram(it->first, it->second);
		if (sck < 0) {
			warnx("could not create socket for '%s:%hu'",
					it->first, it->second);
			close(sck);
			continue;
		}

		int cnt = FetchCount(sck, numtries_, to_);

		if (cnt <= 0) {
			warnx("could not get server count from '%s:%hu'",
					it->first, it->second);
			close(sck);
			continue;
		}

		cnt = FetchList(sck, numtries_, cnt, result, to_);

		if (cnt < 0) {
			warnx("could not get server list from '%s:%hu'",
					it->first, it->second);
			close(sck);
			continue;
		}

		close(sck);
	}
}

int
MasterComm::FetchCount(int sck, int numtries, uint64_t to_us)
{
	int res = -1;
	unsigned char pk[32];
	size_t sz = pg_.MkConnless(pk, sizeof pk, SB_GETCOUNT, NULL, 0);

	while(numtries--) {
		errno = 0;
		ssize_t r = send(sck, pk, sz, MSG_NOSIGNAL);
		if (r == -1) {
			warn("couldn't send()");
			continue;
		}

		unsigned char buf[128];
		r = Util::Recv(sck, buf, sizeof buf, to_us);

		if (r <= 0)
			continue;

		if (!pg_.IsConnless(buf, r)) {
			warnx("not a connless packet");
			continue;
		}

		EClPkts typ = pg_.IdentifyConnless(buf, r);

		if (typ != SB_COUNT) {
			warnx("unexpected reply");
			continue;
		}

		size_t cnt = 0;

		if (!pg_.ParseConnless_SB_COUNT(buf, r, &cnt)) {
			warnx("failed to parse SB_COUNT paket");
			continue;
		}

		res = (int)cnt;

		break;
	}

	return res;
}

int
MasterComm::FetchList(int sck, int numtries, int num_expected,
		vector<string> & result, uint64_t to_us)
{
	int res = -1;

	while(numtries--) {
		int r = TryFetchList(sck, num_expected, result, to_us);
		if (r < 0)
			continue;

		res = r;
		break;
	}

	return res;
}

int
MasterComm::TryFetchList(int sck, int num_expected,
		vector<string> & result, uint64_t to_us)
{
	unsigned char pk[32];
	size_t sz = pg_.MkConnless(pk, sizeof pk, SB_GETLIST, NULL, 0);

	errno = 0;
	ssize_t r = send(sck, pk, sz, MSG_NOSIGNAL);
	if (r == -1) {
		warn("couldn't send()");
		return -1;
	}

	unsigned char buf[4096];

	vector<string> tmp;

	while(tmp.size()+FUZZ < num_expected) {
		r = Util::Recv(sck, buf, sizeof buf, to_us);
		if (r <= 0)
			return -1;

		if (!pg_.IsConnless(buf, r)) {
			warnx("not a connless packet");
			return -1;
		}

		EClPkts typ = pg_.IdentifyConnless(buf, r);

		if (typ != SB_LIST) {
			warnx("unexpected reply");
			return -1;
		}

		if (!pg_.ParseConnless_SB_LIST(buf, r, tmp)) {
			warnx("failed to parse SB_COUNT paket");
			return -1;
		}

	}

	result.insert(result.end(), tmp.begin(), tmp.end());

	return (int)tmp.size();
}

};
