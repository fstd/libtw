/* info.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_INFO_H
#define LIBTW_INFO_H 1

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

namespace tw {
struct ServerInfo;

class InfoComm {
private:
	map<string, ServerInfo> infomap_;


public:
	InfoComm();

	void SetServers(vector<string> const& srvs);

	ServerInfo const& Get(string const& addr) const { return infomap_.at(addr); }
	map<string, ServerInfo> const& GetAll() const { return infomap_; }

	int Refresh();
};

struct ServerInfo {
	string addr_;
	string name_;
	string mod_;
	string map_;
	string ver_;
	int numc_;
	int maxc_;
	int nump_;
	int maxp_;
	int lat_;

	uint64_t tlast_;
};

};

#endif /* LIBTW_INFO_H */
