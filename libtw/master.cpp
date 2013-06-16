/* master.cpp - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <err.h>

extern "C" {
#include <libsrsbsns/addr.h>
}

#include <libtw/master.h>

namespace tw {

MasterComm::MasterComm()
: msrvs_(),
  numtries_(2),
  to_(500000)
{
}

MasterComm::~MasterComm()
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
	

	for(set<pair<char*, unsigned short> >::const_iterator
			it = msrvs_.begin(); it != msrvs_.end(); it++) {

		int sck = addr_connect_socket_dgram(it->first, it->second);
		if (sck < 0) {
			warnx("could not create socket for '%s:%hu'",
					it->first, it->second);
			continue;
		}

		int cnt = FetchCount(sck, numtries_, to_);

		if (cnt <= 0) {
			warnx("could not get server count from '%s:%hu'",
					it->first, it->second);
			continue;
		}
	
		cnt = FetchList(sck, numtries_, result, to_);

		if (cnt <= 0) {
			warnx("could not get server list from '%s:%hu'",
					it->first, it->second);
			continue;
		}

		fprintf(stderr, "got %d servers from '%s:%hu'\n", cnt,
				it->first, it->second);

		close(sck);
	}
}

int
MasterComm::FetchCount(int sck, int numtries, unsigned long to_us)
{
	return -1;
}

int
MasterComm::FetchList(int sck, int numtries, vector<string> & result,
		unsigned long to_us)
{
	return -1;
}

};
