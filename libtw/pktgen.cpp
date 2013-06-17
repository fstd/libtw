/* pktgen.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstring>

#include <arpa/inet.h>
#include <err.h>

#include <libtw/pktgen.h>

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



PktGen::PktGen()
{
}

PktGen::~PktGen()
{
}

size_t
PktGen::MkConnless(unsigned char *buf, size_t bufsz, EClPkts typ,
			const void *extra, size_t extralen)
{
	size_t pklen = clpkt_len[typ];
	if (6 + pklen + extralen > bufsz) {
		warnx("buffer too small for connless packet");
		return 0;
	}

	memcpy(buf+6, clpkt_data[typ], pklen);
	if (extralen)
		memcpy(buf+6+pklen, extra, extralen);

	for(size_t i = 0; i < 6; i++)
		buf[i] = 0xff;

	return 6 + pklen + extralen;
}


bool
PktGen::IsConnless(unsigned char *pk, size_t pklen) const
{
	unsigned char head[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	return memcmp(pk, head, 6) == 0;
}

bool
PktGen::IsControl(unsigned char *pk, size_t pklen) const
{
	/* TODO implement me */
	return false;
}

bool
PktGen::IsSystem(unsigned char *pk, size_t pklen) const
{
	/* TODO implement me */
	return false;
}

EClPkts
PktGen::IdentifyConnless(unsigned char *pk, size_t pklen) const
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

int
PktGen::IdentifyControl(unsigned char *pk, size_t pklen) const
{
	/* TODO implement me */
	return -1;
}

int
PktGen::IdentifyRegular(unsigned char *pk, size_t pklen) const
{
	/* TODO implement me */
	return -1;
}

const char*
PktGen::NameConnless(EClPkts typ) const
{
	return clpkt_names[typ];
}

bool
PktGen::ParseConnless_SB_COUNT(unsigned char *pk, size_t pklen,
			size_t *out_srvcnt) const
{
	if (pklen < 6+8+2)
		return false;
	
	*out_srvcnt = pk[14]*256 + pk[15];
	return true;
}

bool
PktGen::ParseConnless_SB_LIST(unsigned char *pk, size_t pklen,
		vector<string> & result) const
{
	if (pklen < 6+8)
		return false;
	
	if ((pklen-(6+8)) % 18 != 0) {
		warnx("odd SB_LIST packet length: '%zu' (with header)",
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
			warn("inet_ntop");
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

};
