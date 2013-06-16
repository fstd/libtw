/* master.h - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_MASTER_H
#define LIBTW_MASTER_H

#include <vector>
#include <set>
#include <string>

using std::vector;
using std::set;
using std::string;

namespace tw {
class MasterComm {
private:
	set<string> msrvs_;

public:
	MasterComm();
	virtual ~MasterComm();

	/* set master servers, format: "host:port" (host can be FQDN,
	 * IPv4 or [IPv6] address) */
	void SetMasters(vector<string> const& masters);

	/* retrieve list from (all) masters, putting them into
	 * the supplied vector ,,result'' */
	void GetList(vector<string> & result);

};
};

#endif /* LIBTW_MASTER_H */
