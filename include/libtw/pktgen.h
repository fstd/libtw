/* pktgen.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_PKTGEN_H
#define LIBTW_PKTGEN_H

#include <vector>
#include <string>

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

using std::vector;
using std::string;

struct ServerInfo;

class PktGen {
public:
	PktGen();
	virtual ~PktGen();

	size_t MkConnless(unsigned char *pBuf, size_t BufSz, EClPkts typ,
			const void *pData, size_t DataLen);

	size_t MkConnless_SB_GETINFO(unsigned char *pBuf, size_t BufSz,
			unsigned char token);

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

	bool ParseConnless_SB_COUNT(unsigned char *pk, size_t pklen,
			size_t *out_srvcnt) const;

	bool ParseConnless_SB_LIST(unsigned char *pk, size_t pklen,
			vector<string> & result) const;

	bool ParseConnless_SB_INFO(unsigned char *pk, size_t pklen,
			ServerInfo *out_info) const;


};

/* This is cuntpasted from Teeworlds and somewhat mangled,
 * but originally (c) Magnus Auvinen; see README for details */
// variable int packing
class CVariableInt
{
public:
	static unsigned char *Pack(unsigned char *pDst, int i);
	static const unsigned char *Unpack(const unsigned char *pSrc, int *pInOut);
	static long Compress(const void *pSrc, int Size, void *pDst);
	static long Decompress(const void *pSrc, int Size, void *pDst);
};

class CPacker
{
	enum
	{
		PACKER_BUFFER_SIZE=1024*2
	};

	unsigned char m_aBuffer[PACKER_BUFFER_SIZE];
	unsigned char *m_pCurrent;
	unsigned char *m_pEnd;
	int m_Error;
public:
	void Reset();
	void AddInt(int i);
	void AddString(const char *pStr, int Limit);
	void AddRaw(const void *pData, int Size);

	int Size() const { return (int)(m_pCurrent-m_aBuffer); }
	const unsigned char *Data() const { return m_aBuffer; }
	bool Error() const { return m_Error; }
};

class CUnpacker
{
	const unsigned char *m_pStart;
	const unsigned char *m_pCurrent;
	const unsigned char *m_pEnd;
	int m_Error;
public:
	enum
	{
		SANITIZE=1,
		SANITIZE_CC=2,
		SKIP_START_WHITESPACES=4
	};

	void Reset(const void *pData, int Size);
	int GetInt();
	const char *GetString(int SanitizeType = SANITIZE);
	const unsigned char *GetRaw(int Size);
	bool Error() const { return m_Error; }
};


};

#endif /* LIBTW_PKTGEN_H */
