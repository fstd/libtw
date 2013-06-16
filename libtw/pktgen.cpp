/* pktgen.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <initializer_list>

#include <cstring>

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

};
