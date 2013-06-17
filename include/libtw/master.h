/* master.h - (C) 2013, Timo Buhrmester, Learath2
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_MASTER_H
#define LIBTW_MASTER_H

#include <vector>
#include <set>
#include <utility>
#include <string>

#include <libtw/pktgen.h>

using std::vector;
using std::set;
using std::pair;
using std::string;

namespace tw {
class MasterComm {
private:
	set<pair<char*, unsigned short> > msrvs_;
	int numtries_;
	uint64_t to_;

	PktGen pg_;

	int FetchCount(int sck, int numtries, uint64_t to_us);
	int FetchList(int sck, int numtries, int num_expected,
			vector<string> & result, uint64_t to_us);
	int TryFetchList(int sck, int num_expected,
			vector<string> & result, uint64_t to_us);

public:
	MasterComm();

	/* set master servers, format: "host:port" (host can be FQDN,
	 * IPv4 or [IPv6] address) */
	void SetMasters(vector<string> const& masters);

	/* retrieve list from (all) masters, putting them into
	 * the supplied vector ,,result'' */
	void GetList(vector<string> & result);

};
};

#endif /* LIBTW_MASTER_H */
