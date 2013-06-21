/* twprot.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_TWPROT_H
#define LIBTW_TWPROT_H 1

namespace tw {

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

#endif /* LIBTW_TWPROT_H */
