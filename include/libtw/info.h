/* info.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_INFO_H
#define LIBTW_INFO_H 1

#include <string>
#include <vector>
#include <map>

#include <libtw/proto_connless.h>

using std::string;
using std::vector;
using std::map;

namespace tw {
struct ServerInfo;

class InfoComm {
private:
	map<string, ServerInfo> infomap_;
	ConnlessProtoUnit pg_;
	unsigned char tok_;
	uint64_t to_;

	int RefreshChunk(int sck, unsigned char tok,
			vector<string>::const_iterator start,
			size_t num);


public:
	InfoComm();

	void SetServers(vector<string> const& srvs);

	ServerInfo const& Get(string const& addr) const { return infomap_.at(addr); }
	map<string, ServerInfo> const& GetAll() const { return infomap_; }

	int Refresh();
};


struct PlayerInfo {
	string name_;
	string clan_;
	int country_;
	int score_;
	bool player_;

	PlayerInfo(string const& name, string const& clan, int country,
			int score, bool player)
	: name_(name),
	  clan_(clan),
	  country_(country),
	  score_(score),
	  player_(player)
	{
	}

	void Dump() const;

};

struct ServerInfo {
	string addr_;
	string name_;
	string mod_;
	string map_;
	string ver_;
	int flg_;
	int numc_;
	int maxc_;
	int nump_;
	int maxp_;

	bool on_;
	uint64_t tsend_;
	uint64_t trecv_;

	vector<PlayerInfo> clt_;

	void Dump() const;
};

};

#endif /* LIBTW_INFO_H */
