/* master.cpp - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstdio>

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
	for(vector<string>::const_iterator it = masters.begin();
			it != masters.end(); it++)
		msrvs_.insert(*it);
}

void
MasterComm::GetList(vector<string> & result)
{
	if (msrvs_.size() == 0)
		return;
	
	//...
}

};
