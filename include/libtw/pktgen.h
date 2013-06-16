/* pktgen.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_PKTGEN_H
#define LIBTW_PKTGEN_H

#include <cstddef>

namespace tw {
class PktGen {
public:
	PktGen();
	virtual ~PktGen();

	size_t MkConnless(unsigned char *pBuf, size_t BufSz,
			const void *pData, size_t DataLen);

	/* tell apart connless from regular packets */
	bool IsConnless(unsigned char *pk, size_t pklen) const;

	/* if regular, tell apart control and normal packets */
	bool IsControl(unsigned char *pk, size_t pklen) const;

	/* if normal-regular, tell apart system/user messages */
	bool IsSystem(unsigned char *pk, size_t pklen) const;

	/* for connless packets, return type of connless packet */
	int IdentifyConnless(unsigned char *pk, size_t pklen) const;

	/* for control-regular, return type control packet */
	int IdentifyControl(unsigned char *pk, size_t pklen) const;

	/* for normal-regular (user and system), return msgtype */
	int IdentifyRegular(unsigned char *pk, size_t pklen) const;
};
};

#endif /* LIBTW_PKTGEN_H */
