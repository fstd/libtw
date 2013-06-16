/* pktgen.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstring>

#include <err.h>

#include <libtw/pktgen.h>

namespace tw {

PktGen::PktGen()
{
}

PktGen::~PktGen()
{
}

size_t
PktGen::MkConnless(unsigned char *pBuf, size_t BufSz,
			const void *pData, size_t DataLen)
{
	if (DataLen+6 > BufSz) {
		warnx("buffer too small for connless packet");
		return 0;
	}

	memcpy(pBuf+6, pData, DataLen);
	for(size_t i = 0; i < 6; i++)
		pBuf[i] = 0xff;

	return DataLen+6;
}

};
