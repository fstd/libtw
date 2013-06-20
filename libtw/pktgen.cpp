/* pktgen.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstring>

#include <arpa/inet.h>
#include <err.h>

#include <libtw/info.h>
#include <libtw/util.h>
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


size_t
PktGen::MkConnless_SB_GETINFO(unsigned char *pBuf, size_t BufSz,
		unsigned char token)
{
	return MkConnless(pBuf, BufSz, SB_GETINFO, &token, 1);
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

bool
PktGen::ParseConnless_SB_INFO(unsigned char *pk, size_t pklen,
		ServerInfo *out_info) const
{
	CUnpacker Up;
	Up.Reset(pk+6+8, pklen - (6+8));

	(void)Up.GetString(); //we did the token already
	out_info->ver_ = Up.GetString();
	if (strncmp(out_info->ver_.c_str(), "0.6", 3) != 0) {
		warnx("wrong version (%s)", out_info->ver_.c_str());
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
		warnx("failed to parse for '%s'", out_info->addr_.c_str());
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
			warnx("failed to parse for '%s' (%d)",
					out_info->addr_.c_str(), i);
			Util::hexdump(pk, pklen, "errorneous SB_INFO");
			return false;
		}

		out_info->clt_.push_back(PlayerInfo(name, clan, country,
				score, player));
	}

	return true;
}


/* This is cuntpasted from Teeworlds and somewhat mangled,
 * but originally (c) Magnus Auvinen; see README for details */
void CPacker::Reset()
{
	m_Error = 0;
	m_pCurrent = m_aBuffer;
	m_pEnd = m_pCurrent + PACKER_BUFFER_SIZE;
}

void CPacker::AddInt(int i)
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

void CPacker::AddString(const char *pStr, int Limit)
{
	if(m_Error)
		return;

	//
	if(Limit > 0)
	{
		while(*pStr && Limit != 0)
		{
			*m_pCurrent++ = *pStr++;
			Limit--;

			if(m_pCurrent >= m_pEnd)
			{
				m_Error = 1;
				break;
			}
		}
		*m_pCurrent++ = 0;
	}
	else
	{
		while(*pStr)
		{
			*m_pCurrent++ = *pStr++;

			if(m_pCurrent >= m_pEnd)
			{
				m_Error = 1;
				break;
			}
		}
		*m_pCurrent++ = 0;
	}
}

void CPacker::AddRaw(const void *pData, int Size)
{
	if(m_Error)
		return;

	if(m_pCurrent+Size >= m_pEnd)
	{
		m_Error = 1;
		return;
	}

	const unsigned char *pSrc = (const unsigned char *)pData;
	while(Size)
	{
		*m_pCurrent++ = *pSrc++;
		Size--;
	}
}


void CUnpacker::Reset(const void *pData, int Size)
{
	m_Error = 0;
	m_pStart = (const unsigned char *)pData;
	m_pEnd = m_pStart + Size;
	m_pCurrent = m_pStart;
}

int CUnpacker::GetInt()
{
	if(m_Error)
		return 0;

	if(m_pCurrent >= m_pEnd)
	{
		m_Error = 1;
		return 0;
	}

	int i;
	m_pCurrent = CVariableInt::Unpack(m_pCurrent, &i);
	if(m_pCurrent > m_pEnd)
	{
		m_Error = 1;
		return 0;
	}
	return i;
}

const char *CUnpacker::GetString(int SanitizeType)
{
	if(m_Error || m_pCurrent >= m_pEnd)
		return "";

	char *pPtr = (char *)m_pCurrent;
	while(*m_pCurrent) // skip the string
	{
		m_pCurrent++;
		if(m_pCurrent == m_pEnd)
		{
			m_Error = 1;;
			return "";
		}
	}
	m_pCurrent++;

	// sanitize all strings
	return pPtr;
}

const unsigned char *CUnpacker::GetRaw(int Size)
{
	const unsigned char *pPtr = m_pCurrent;
	if(m_Error)
		return 0;

	// check for nasty sizes
	if(Size < 0 || m_pCurrent+Size > m_pEnd)
	{
		m_Error = 1;
		return 0;
	}

	// "unpack" the data
	m_pCurrent += Size;
	return pPtr;
}

// Format: ESDDDDDD EDDDDDDD EDD... Extended, Data, Sign
unsigned char *CVariableInt::Pack(unsigned char *pDst, int i)
{
	*pDst = (i>>25)&0x40; // set sign bit if i<0
	i = i^(i>>31); // if(i<0) i = ~i

	*pDst |= i&0x3F; // pack 6bit into dst
	i >>= 6; // discard 6 bits
	if(i)
	{
		*pDst |= 0x80; // set extend bit
		while(1)
		{
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

const unsigned char *CVariableInt::Unpack(const unsigned char *pSrc, int *pInOut)
{
	int Sign = (*pSrc>>6)&1;
	*pInOut = *pSrc&0x3F;

	do
	{
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


long CVariableInt::Decompress(const void *pSrc_, int Size, void *pDst_)
{
	const unsigned char *pSrc = (unsigned char *)pSrc_;
	const unsigned char *pEnd = pSrc + Size;
	int *pDst = (int *)pDst_;
	while(pSrc < pEnd)
	{
		pSrc = CVariableInt::Unpack(pSrc, pDst);
		pDst++;
	}
	return (long)((unsigned char *)pDst-(unsigned char *)pDst_);
}

long CVariableInt::Compress(const void *pSrc_, int Size, void *pDst_)
{
	int *pSrc = (int *)pSrc_;
	unsigned char *pDst = (unsigned char *)pDst_;
	Size /= 4;
	while(Size)
	{
		pDst = CVariableInt::Pack(pDst, *pSrc);
		Size--;
		pSrc++;
	}
	return (long)(pDst-(unsigned char *)pDst_);
}
};
