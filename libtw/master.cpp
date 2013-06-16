/* master.cpp - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
#include <libsrsbsns/addr.h>
}

#include <libtw/master.h>

namespace tw {

MasterComm::MasterComm()
: msrvs_()
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
		fprintf(stderr, "listing '%s:%hu'\n",
				it->first, it->second);
	}
}

};
