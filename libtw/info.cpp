/* info.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstring>
#include <random>
#include <algorithm>

#include <ctime>

#include <unistd.h>
#include <pthread.h>

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
: infomap_(), pg_(), tok_(0), chunksz_(100), to_(2000000), reqdelay_(0)
{
}


void
InfoComm::SetPolicy(size_t chunksz, uint64_t to_us, uint64_t reqdelay)
{
	chunksz_ = chunksz;
	to_ = to_us;
	reqdelay_ = reqdelay;
}

void
InfoComm::SetServers(vector<string> const& srvs)
{
	infomap_.clear();

	for(vector<string>::const_iterator it = srvs.begin();
			it != srvs.end(); it++) {

		bool is64 = strncmp(it->c_str(), "64!", 3) == 0;

		string key = is64 ? string(it->c_str()+3) : *it;
		if (is64) {
			infomap_[key].addr_ = string(it->c_str()+3);
			infomap_[key].ext64 = true;
		} else {
			infomap_[key].addr_ = *it;
			infomap_[key].ext64 = false;
		}

		infomap_[key].tsend_ = infomap_[key].trecv_ = 0;
		infomap_[key].on_ = false;
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

	if (pthread_mutex_init(&mtx_, NULL) != 0) {
		WX("couldn't make mutex");
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

	//std::shuffle(std::begin(addrs), std::end(addrs), std::default_random_engine(time(NULL)));
	std::random_shuffle(addrs.begin(), addrs.end());

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

	for (auto it = infomap_.begin(); it != infomap_.end(); it++) {
		if (it->second.on_ && it->second.ext64
		    && it->second.numc_ != it->second.clt_.size())
			it->second.on_ = false;
			
	}

	pthread_mutex_destroy(&mtx_);
	return suc;
}


void *tfun(void *v) {
	static_cast<InfoComm*>(v)->RefreshChunk_T();
	return NULL;
}


void
InfoComm::RefreshChunk_T()
{
	unsigned char pk[16];
	unsigned char pk64[16];
	size_t sz = pg_.MkConnless_SB_GETINFO(pk, sizeof pk, td_tok);
	size_t sz64 = pg_.MkConnless_SB_GETINFO64(pk64, sizeof pk64, td_tok);

	for(size_t i = 0; i < td_num; ((td_start++), (i++))) {
		uint64_t tsend = Util::tstamp();
		ssize_t r;

		bool is64 = strncmp(td_start->c_str(), "64!", 3) == 0;

		string key = is64 ? string(td_start->c_str()+3) : *td_start;

		WVX( "sending to '%s'", key.c_str());
		if (is64)
			r = Util::Send(td_sck, pk64, sz64, key.c_str());
		else
			r = Util::Send(td_sck, pk, sz, key.c_str());

		if (r == -1) {
			WX("Util::Send() failed (%zu bytes to %s)", sz, key.c_str());
			continue;
		}

		infomap_[key].tsend_ = tsend;

		if (reqdelay_)
			usleep(reqdelay_);
	}

	if (pthread_mutex_lock(&mtx_) != 0) {
		WX("failed to lock mutex");
	}
	done_ = true;
	pthread_mutex_unlock(&mtx_);
}

int
InfoComm::RefreshChunk(int sck, unsigned char tok,
		vector<string>::const_iterator start, size_t num)
{
	td_sck = sck;
	td_tok = tok;
	td_start = start;
	td_num = num;

	for(size_t i = 0; i < num; ((start++), (i++))) {
		bool is64 = strncmp(start->c_str(), "64!", 3) == 0;
		string key = is64 ? string(start->c_str()+3) : *start;
		infomap_[key].on_ = false;
	}

	done_ = false;

	pthread_t thr;
	if (pthread_create(&thr, NULL, tfun, this) != 0) {
		WX("failed to make thread");
		return 0;
	}

	int suc = 0;

	bool dieplx = false;

	for(;;) {
		char from[128] = {0};
		unsigned char buf[2048];
		ssize_t r = Util::Recv(sck, buf, sizeof buf, to_, 1000,
				from, sizeof from);

		if (r < 0)
			break;
		if (r == 0) {
			if (dieplx)
				break;
			bool d;
			if (pthread_mutex_lock(&mtx_) != 0) {
				WX("failed to lock mutex!");
				return 0;
			}
			d = done_;
			pthread_mutex_unlock(&mtx_);
			if (d)
				dieplx = true;

			continue;
		}

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

		info->reacted_ = true;

		if (typ == SB_INFO) {
			if (!pg_.ParseConnless_SB_INFO(buf, r, info))
				continue;
			suc++;
		} else if (typ == SB_INFO64) {
			if (!pg_.ParseConnless_SB_INFO64(buf, r, info))
				continue;

			if (info->clt_.size() == info->numc_) {
				suc++;
			}
		}

		info->trecv_ = trecv;
		info->on_ = true;
	}

	pthread_join(thr, NULL);
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
