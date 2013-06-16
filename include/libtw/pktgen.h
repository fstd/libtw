/* pktgen.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_PKTGEN_H
#define LIBTW_PKTGEN_H

#include <cstddef>
#define Y(A...) A

namespace tw {


#define CONNLESS_PACKETS                                        \
X(SB_HEARTBEAT,        Y({255, 255, 255, 255, 'b', 'e', 'a', '2'})) \
X(SB_GETLIST,          Y({255, 255, 255, 255, 'r', 'e', 'q', '2'})) \
X(SB_LIST,             Y({255, 255, 255, 255, 'l', 'i', 's', '2'})) \
X(SB_GETCOUNT,         Y({255, 255, 255, 255, 'c', 'o', 'u', '2'})) \
X(SB_COUNT,            Y({255, 255, 255, 255, 's', 'i', 'z', '2'})) \
X(SB_GETINFO,          Y({255, 255, 255, 255, 'g', 'i', 'e', '3'})) \
X(SB_INFO,             Y({255, 255, 255, 255, 'i', 'n', 'f', '3'})) \
X(SB_FWCHECK,          Y({255, 255, 255, 255, 'f', 'w', '?', '?'})) \
X(SB_FWRESPONSE,       Y({255, 255, 255, 255, 'f', 'w', '!', '!'})) \
X(SB_FWOK,             Y({255, 255, 255, 255, 'f', 'w', 'o', 'k'})) \
X(SB_FWERROR,          Y({255, 255, 255, 255, 'f', 'w', 'e', 'r'})) \
X(SB_HEARTBEAT_LEGACY, Y({255, 255, 255, 255, 'b', 'e', 'a', 't'})) \
X(SB_GETLIST_LEGACY,   Y({255, 255, 255, 255, 'r', 'e', 'q', 't'})) \
X(SB_LIST_LEGACY,      Y({255, 255, 255, 255, 'l', 'i', 's', 't'})) \
X(SB_GETCOUNT_LEGACY,  Y({255, 255, 255, 255, 'c', 'o', 'u', 'n'})) \
X(SB_COUNT_LEGACY,     Y({255, 255, 255, 255, 's', 'i', 'z', 'e'})) \
X(VS_GETVERSION,       Y({255, 255, 255, 255, 'v', 'e', 'r', 'g'})) \
X(VS_VERSION,          Y({255, 255, 255, 255, 'v', 'e', 'r', 's'})) \
X(VS_GETMAPLIST,       Y({255, 255, 255, 255, 'v', 'm', 'l', 'g'})) \
X(VS_MAPLIST,          Y({255, 255, 255, 255, 'v', 'm', 'l', 's'})) \
X(INVALID_CLPKT,       Y({254, 255, 255, 255, 'i', 'n', 'v', 'l'}))

#define X(a, b) a,
enum EClPkts {
	CONNLESS_PACKETS
};
#undef X

class PktGen {
public:
	PktGen();
	virtual ~PktGen();

	size_t MkConnless(unsigned char *pBuf, size_t BufSz, EClPkts typ,
			const void *pData, size_t DataLen);

	/* tell apart connless from regular packets */
	bool IsConnless(unsigned char *pk, size_t pklen) const;

	/* if regular, tell apart control and normal packets */
	bool IsControl(unsigned char *pk, size_t pklen) const;

	/* if normal-regular, tell apart system/user messages */
	bool IsSystem(unsigned char *pk, size_t pklen) const;

	/* for connless packets, return type of connless packet */
	EClPkts IdentifyConnless(unsigned char *pk, size_t pklen) const;

	/* for control-regular, return type control packet */
	int IdentifyControl(unsigned char *pk, size_t pklen) const;

	/* for normal-regular (user and system), return msgtype */
	int IdentifyRegular(unsigned char *pk, size_t pklen) const;

	const char *NameConnless(EClPkts typ) const;

};

};

#endif /* LIBTW_PKTGEN_H */
