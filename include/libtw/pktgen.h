/* pktgen.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_PKTGEN_H
#define LIBTW_PKTGEN_H

namespace tw {
class PktGen {
public:
	PktGen();
	virtual ~PktGen();

	size_t MkConnless(unsigned char *pBuf, size_t BufSz,
			const void *pData, size_t DataLen);

};
};

#endif /* LIBTW_PKTGEN_H */
