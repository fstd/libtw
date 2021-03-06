/* proto_connless.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstring>

#include <arpa/inet.h>
#include <err.h>

#include <libtw/info.h>
#include <libtw/util.h>

#include "twprot.h"
#include "debug.h"

#include <libtw/proto_connless.h>

namespace tw {

#define X(a, b) b,
static const unsigned char clpkt_data[][8] = {
	CONNLESS_PACKETS
};
#undef X

#define X(a, b) 8,
static const size_t clpkt_len[] = {
	CONNLESS_PACKETS
};
#undef X

#define X(a, b) #a,
static const char *clpkt_names[] = {
	CONNLESS_PACKETS
};
#undef X


ConnlessProtoUnit::ConnlessProtoUnit()
{
}

ConnlessProtoUnit::~ConnlessProtoUnit()
{
}

size_t
ConnlessProtoUnit::MkConnless(unsigned char *buf, size_t bufsz, EClPkts typ,
			const void *extra, size_t extralen)
{
	size_t pklen = clpkt_len[typ];
	if (6 + pklen + extralen > bufsz) {
		WX("buffer too small for connless packet");
		return 0;
	}

	memcpy(buf+6, clpkt_data[typ], pklen);
	if (extralen)
		memcpy(buf+6+pklen, extra, extralen);

	for(size_t i = 0; i < 6; i++)
		buf[i] = 0xff;

	return 6 + pklen + extralen;
}


size_t
ConnlessProtoUnit::MkConnless_SB_GETINFO(unsigned char *pBuf, size_t BufSz,
		unsigned char token)
{
	return MkConnless(pBuf, BufSz, SB_GETINFO, &token, 1);
}

bool
ConnlessProtoUnit::IsConnless(unsigned char *pk, size_t pklen) const
{
	unsigned char head[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	return memcmp(pk, head, 6) == 0;
}

EClPkts
ConnlessProtoUnit::IdentifyConnless(unsigned char *pk, size_t pklen) const
{
	if (pklen < 14)
		return INVALID_CLPKT;
#define X(a, b) { unsigned char tmp[] b; \
                  if (memcmp(tmp, pk+6, 8) == 0) \
                          return a; }
	CONNLESS_PACKETS
#undef X
	return INVALID_CLPKT;
}

const char*
ConnlessProtoUnit::NameConnless(EClPkts typ) const
{
	return clpkt_names[typ];
}

bool
ConnlessProtoUnit::ParseConnless_SB_COUNT(unsigned char *pk, size_t pklen,
			size_t *out_srvcnt) const
{
	if (pklen < 6+8+2)
		return false;

	*out_srvcnt = pk[14]*256 + pk[15];
	return true;
}

bool
ConnlessProtoUnit::ParseConnless_SB_LIST(unsigned char *pk, size_t pklen,
		vector<string> & result) const
{
	if (pklen < 6+8)
		return false;

	if ((pklen-(6+8)) % 18 != 0) {
		WX("odd SB_LIST packet length: '%zu' (with header)",
				pklen);
		return -1;
	}

	unsigned char ip4map[] = {0x00, 0x00, 0x00, 0x00,
	                          0x00, 0x00, 0x00, 0x00,
	                          0x00, 0x00, 0xff, 0xff};

	for(size_t i = 6+8; i < pklen; i += 18) {
		bool ip4 = memcmp(pk+i, ip4map, sizeof ip4map) == 0;
		char addr[48] = {0};
		errno = 0;
		if (!inet_ntop(ip4?AF_INET:AF_INET6, pk+i+(ip4?12:0),
				addr + (ip4?0:1), sizeof addr - (ip4?0:1))){
			W("inet_ntop");
			continue;
		}

		if (!ip4) {
			addr[0] = '[';
			addr[strlen(addr)] = ']';
		}

		unsigned short port = (pk[i+16]*256) + pk[i+17];
		sprintf(addr+strlen(addr), ":%hu", port);

		result.push_back(string(addr));
	}

	return true;
}

bool
ConnlessProtoUnit::ParseConnless_SB_INFO(unsigned char *pk, size_t pklen,
		ServerInfo *out_info) const
{
	CUnpacker Up;
	Up.Reset(pk+6+8, pklen - (6+8));

	(void)Up.GetString(); //we did the token already
	out_info->ver_ = Up.GetString();
	if (strncmp(out_info->ver_.c_str(), "0.6", 3) != 0) {
		WX("wrong version (%s)", out_info->ver_.c_str());
		return false;
	}

	out_info->name_ = Up.GetString();
	out_info->map_ = Up.GetString();
	out_info->mod_ = Up.GetString();
	out_info->flg_ = (int)strtol(Up.GetString(), NULL, 10);
	out_info->nump_ = (int)strtol(Up.GetString(), NULL, 10);
	out_info->maxp_ = (int)strtol(Up.GetString(), NULL, 10);
	out_info->numc_ = (int)strtol(Up.GetString(), NULL, 10);
	out_info->maxc_ = (int)strtol(Up.GetString(), NULL, 10);

	if (Up.Error()) {
		WX("failed to parse for '%s'", out_info->addr_.c_str());
		Util::hexdump(pk, pklen, "errorneous SB_INFO");
		return false;
	}

	for(int i = 0; i < out_info->numc_; i++)
	{
		string name(Up.GetString());
		string clan(Up.GetString());
		int country = (int)strtol(Up.GetString(), NULL, 0);
		int score = (int)strtol(Up.GetString(), NULL, 0);
		bool player = (bool)strtol(Up.GetString(), NULL, 0);

		if (Up.Error()) {
			WX("failed to parse for '%s' (%d)",
					out_info->addr_.c_str(), i);
			Util::hexdump(pk, pklen, "errorneous SB_INFO");
			return false;
		}

		out_info->clt_.push_back(PlayerInfo(name, clan, country,
				score, player));
	}

	return true;
}


};
