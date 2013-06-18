/* info.cpp - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <libtw/info.h>

namespace tw {

InfoComm::InfoComm()
: infomap_()
{
}

void
InfoComm::SetServers(vector<string> const& srvs)
{
	infomap_.clear();

	for(vector<string>::const_iterator it = srvs.begin();
			it != srvs.end(); it++)
		infomap_[*it].tlast_ = 0;
}

int
InfoComm::Refresh()
{
	return 0;
}

};
