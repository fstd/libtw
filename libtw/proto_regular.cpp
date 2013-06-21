/* proto_regular.cpp - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <cstring>

#include "debug.h"

#include <libtw/proto_regular.h>

namespace tw {


RegularProtoUnit::RegularProtoUnit()
{
}

RegularProtoUnit::~RegularProtoUnit()
{
}

bool
RegularProtoUnit::IsRegular(unsigned char *pk, size_t pklen) const
{
	unsigned char head[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	if (memcmp(pk, head, 6) == 0)
		return false; //connless
	/* do moar checks or leave it to identify()? */
	return true;
}


bool
RegularProtoUnit::IsControl(unsigned char *pk, size_t pklen) const
{
	/* TODO implement me */
	return false;
}

bool
RegularProtoUnit::IsSystem(unsigned char *pk, size_t pklen) const
{
	/* TODO implement me */
	return false;
}

int
RegularProtoUnit::IdentifyControl(unsigned char *pk, size_t pklen) const
{
	/* TODO implement me */
	return -1;
}

int
RegularProtoUnit::IdentifyRegular(unsigned char *pk, size_t pklen) const
{
	/* TODO implement me */
	return -1;
}

};
