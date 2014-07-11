/* twprot.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <err.h>

#include "twprot.h"

namespace tw {

/* This is cuntpasted from Teeworlds and somewhat mangled,
 * but originally (c) Magnus Auvinen; see README for details */
void
CPacker::Reset()
{
	m_Error = 0;
	m_pCurrent = m_aBuffer;
	m_pEnd = m_pCurrent + PACKER_BUFFER_SIZE;
}

void 
CPacker::AddInt(int i)
{
	if(m_Error)
		return;

	// make sure that we have space enough
	if(m_pEnd - m_pCurrent < 6)
	{
		warnx("out of space");
		m_Error = 1;
	}
	else
		m_pCurrent = CVariableInt::Pack(m_pCurrent, i);
}

void 
CPacker::AddString(const char *pStr, int Limit)
{
	if(m_Error)
		return;

	if(Limit > 0) {
		while(*pStr && Limit != 0) {
			*m_pCurrent++ = *pStr++;
			Limit--;

			if(m_pCurrent >= m_pEnd) {
				m_Error = 1;
				break;
			}
		}
		*m_pCurrent++ = 0;
	} else {
		while(*pStr) {
			*m_pCurrent++ = *pStr++;

			if(m_pCurrent >= m_pEnd) {
				m_Error = 1;
				break;
			}
		}
		*m_pCurrent++ = 0;
	}
}

void 
CPacker::AddRaw(const void *pData, int Size)
{
	if(m_Error)
		return;

	if(m_pCurrent+Size >= m_pEnd) {
		m_Error = 1;
		return;
	}

	const unsigned char *pSrc = (const unsigned char *)pData;
	while(Size) {
		*m_pCurrent++ = *pSrc++;
		Size--;
	}
}


void 
CUnpacker::Reset(const void *pData, int Size)
{
	m_Error = 0;
	m_pStart = (const unsigned char *)pData;
	m_pEnd = m_pStart + Size;
	m_pCurrent = m_pStart;
}

int 
CUnpacker::GetInt()
{
	if(m_Error)
		return 0;

	if(m_pCurrent >= m_pEnd) {
		m_Error = 1;
		return 0;
	}

	int i;
	m_pCurrent = CVariableInt::Unpack(m_pCurrent, &i);
	if(m_pCurrent > m_pEnd) {
		m_Error = 1;
		return 0;
	}
	return i;
}

const char *
CUnpacker::GetString(int SanitizeType)
{
	if(m_Error || m_pCurrent >= m_pEnd) {
		m_Error = 1;
		return "";
	}

	char *pPtr = (char *)m_pCurrent;
	while(*m_pCurrent) { // skip the string
		m_pCurrent++;
		if(m_pCurrent == m_pEnd) {
			m_Error = 1;;
			return "";
		}
	}
	m_pCurrent++;

	return pPtr;
}

const unsigned char *
CUnpacker::GetRaw(int Size)
{
	const unsigned char *pPtr = m_pCurrent;
	if(m_Error)
		return 0;

	// check for nasty sizes
	if(Size < 0 || m_pCurrent+Size > m_pEnd) {
		m_Error = 1;
		return 0;
	}

	// "unpack" the data
	m_pCurrent += Size;
	return pPtr;
}

// Format: ESDDDDDD EDDDDDDD EDD... Extended, Data, Sign
unsigned char *
CVariableInt::Pack(unsigned char *pDst, int i)
{
	*pDst = (i>>25)&0x40; // set sign bit if i<0
	i = i^(i>>31); // if(i<0) i = ~i

	*pDst |= i&0x3F; // pack 6bit into dst
	i >>= 6; // discard 6 bits
	if(i) {
		*pDst |= 0x80; // set extend bit
		while(1) {
			pDst++;
			*pDst = i&(0x7F); // pack 7bit
			i >>= 7; // discard 7 bits
			*pDst |= (i!=0)<<7; // set extend bit (may branch)
			if(!i)
				break;
		}
	}

	pDst++;
	return pDst;
}

const unsigned char *
CVariableInt::Unpack(const unsigned char *pSrc, int *pInOut)
{
	int Sign = (*pSrc>>6)&1;
	*pInOut = *pSrc&0x3F;

	do {
		if(!(*pSrc&0x80)) break;
		pSrc++;
		*pInOut |= (*pSrc&(0x7F))<<(6);

		if(!(*pSrc&0x80)) break;
		pSrc++;
		*pInOut |= (*pSrc&(0x7F))<<(6+7);

		if(!(*pSrc&0x80)) break;
		pSrc++;
		*pInOut |= (*pSrc&(0x7F))<<(6+7+7);

		if(!(*pSrc&0x80)) break;
		pSrc++;
		*pInOut |= (*pSrc&(0x7F))<<(6+7+7+7);
	} while(0);

	pSrc++;
	*pInOut ^= -Sign; // if(sign) *i = ~(*i)
	return pSrc;
}


long 
CVariableInt::Decompress(const void *pSrc_, int Size, void *pDst_)
{
	const unsigned char *pSrc = (unsigned char *)pSrc_;
	const unsigned char *pEnd = pSrc + Size;
	int *pDst = (int *)pDst_;
	while(pSrc < pEnd) {
		pSrc = CVariableInt::Unpack(pSrc, pDst);
		pDst++;
	}
	return (long)((unsigned char *)pDst-(unsigned char *)pDst_);
}

long 
CVariableInt::Compress(const void *pSrc_, int Size, void *pDst_)
{
	int *pSrc = (int *)pSrc_;
	unsigned char *pDst = (unsigned char *)pDst_;
	Size /= 4;
	while(Size) {
		pDst = CVariableInt::Pack(pDst, *pSrc);
		Size--;
		pSrc++;
	}
	return (long)(pDst-(unsigned char *)pDst_);
}

};
