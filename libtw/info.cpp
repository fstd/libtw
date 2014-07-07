/* info.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstring>

#include <unistd.h>

#include <err.h>

extern "C" {
#include <libsrsbsns/addr.h>
}

#include <libtw/info.h>
#include <libtw/util.h>

#include "twprot.h"
#include "debug.h"


namespace tw {

InfoComm::InfoComm()
: infomap_(), pg_(), tok_(0), chunksz_(100), to_(2000000)
{
}


void
InfoComm::SetPolicy(size_t chunksz, uint64_t to_us)
{
	chunksz_ = chunksz;
	to_ = to_us;
}

void
InfoComm::SetServers(vector<string> const& srvs)
{
	infomap_.clear();

	for(vector<string>::const_iterator it = srvs.begin();
			it != srvs.end(); it++) {

		if (strncmp(it->c_str(), "64!", 3) == 0) {
			infomap_[it->c_str()+3].addr_ = string(it->c_str()+3);
			infomap_[it->c_str()+3].ext64 = true;
		} else {
			infomap_[*it].addr_ = *it;
			infomap_[*it].ext64 = false;
		}

		infomap_[*it].tsend_ = infomap_[*it].trecv_ = 0;
		infomap_[*it].on_ = false;
	}
}

int
InfoComm::Refresh()
{
	/*broadcast since we might add LAN support some day*/
	int sck = addr_bind_socket_dgram_p("0.0.0.0", 0, true, true, NULL, NULL, to_ ,to_);

	if (sck < 0) {
		WX("couldn't make socket");
		return -1;
	}

	int suc = 0;

	vector<string> addrs;
	for(map<string, ServerInfo>::const_iterator it = infomap_.begin();
			it != infomap_.end(); it++) {
		if (it->second.ext64)
			addrs.push_back(string("64!") + it->first);
		else
			addrs.push_back(it->first);
	}

	for(size_t i = 0; i < addrs.size(); i += chunksz_) {
		size_t n = i+chunksz_ <= addrs.size()
				?  chunksz_ : addrs.size() - i;

		int r = RefreshChunk(sck, tok_, addrs.begin() + i, n);
		if (r < 0)
			WX("failed to refresh chunk\n");
		else
			suc += r;
	}

	close(sck);

	return suc;
}

int
InfoComm::RefreshChunk(int sck, unsigned char tok,
		vector<string>::const_iterator start, size_t num)
{
	int suc = 0;
	unsigned char pk[16];
	unsigned char pk64[16];
	size_t sz = pg_.MkConnless_SB_GETINFO(pk, sizeof pk, tok);
	size_t sz64 = pg_.MkConnless_SB_GETINFO64(pk64, sizeof pk64, tok);

	for(size_t i = 0; i < num; ((start++), (i++))) {
		uint64_t tsend = Util::tstamp();
		ssize_t r;
		if (strncmp(start->c_str(), "64!", 3) == 0)
			r = Util::Send(sck, pk64, sz64, start->c_str() + 3);
		else
			r = Util::Send(sck, pk, sz, start->c_str());

		if (r == -1) {
			WX("Util::Send() failed (%zu bytes to %s)", sz, start->c_str());
			continue;
		}

		infomap_[*start].tsend_ = tsend;
		infomap_[*start].on_ = false;
	}

	for(;;) {
		char from[128] = {0};
		unsigned char buf[2048];
		ssize_t r = Util::Recv(sck, buf, sizeof buf, to_, 1000,
				from, sizeof from);

		if (r <= 0)
			break;

		uint64_t trecv = Util::tstamp();

		//WX("received %zd bytes from '%s'", r, from);

		if (!pg_.IsConnless(buf, r)) {
			WX("not a connless packet (%s)", from);
			Util::hexdump(buf, r, "not a connless");
			continue;
		}

		EClPkts typ = pg_.IdentifyConnless(buf, r);

		if (typ != SB_INFO && typ != SB_INFO64) {
			WX("unexpected reply '%s'", pg_.NameConnless(typ));
			continue;
		}

		if (!infomap_.count(from)) {
			WX("reply from '%s' whom we don't know!", from);
			continue;
		}

		ServerInfo *info = &infomap_[from];

		CUnpacker Up; //peek inside
		Up.Reset(buf+6+8, r - (6+8));
		int tk = (int)strtol(Up.GetString(), NULL, 10);

		if (tk != tok) {
			WX("wrong token");
			continue;
		}

		if (typ == SB_INFO) {
			if (!pg_.ParseConnless_SB_INFO(buf, r, info))
				continue;
			suc++;
		} else if (typ == SB_INFO64) {
			if (!pg_.ParseConnless_SB_INFO64(buf, r, info))
				continue;

			if (info->clt_.size() == info->numc_) {
				WVX("meep");
				suc++;
			}
		}

		info->trecv_ = trecv;
		info->on_ = true;
	}

	return suc;
}

void ServerInfo::Dump() const
{
	fprintf(stderr, "[%s %s %s %s '%s' 0x%x %d/%d %d/%d]\n",
			addr_.c_str(), ver_.c_str(), mod_.c_str(),
			map_.c_str(), name_.c_str(), flg_, numc_, maxc_,
			nump_, maxp_);
	for(vector<PlayerInfo>::const_iterator it = clt_.begin();
			it != clt_.end(); it++) {
		it->Dump();

	}

}

void PlayerInfo::Dump() const
{
	fprintf(stderr, "\t%s'%s' (%s) %d %d\n", player_?"":"(spec) ",
			name_.c_str(), clan_.c_str(), country_, score_);

}

};
