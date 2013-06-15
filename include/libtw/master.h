/* master.h - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_MASTER_H
#define LIBTW_MASTER_H

namespace tw {
class MasterComm {
public:
	/* set master servers, format: "host:port" (host can be FQDN,
	 * IPv4 or [IPv6] address) */
	virtual void MasterComm::SetMasters(vector<string> const& masters) = 0;

	/* retrieve list from (all) masters, putting them into
	 * the supplied vector ,,result'' */
	virtual void MasterComm::GetList(vector<string> & result) = 0;

	virtual ~MasterComm();
};
};

#endif /* LIBTW_MASTER_H */
