/* proto_regular.h - (C) 2013, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#ifndef LIBTW_PROTO_REGULAR_H
#define LIBTW_PROTO_REGULAR_H 1

#include <cstddef>

namespace tw {

class RegularProtoUnit {
public:
	RegularProtoUnit();
	virtual ~RegularProtoUnit();

	/* tell apart connless from regular packets */
	bool IsRegular(unsigned char *pk, size_t pklen) const;

	/* if regular, tell apart control and normal packets */
	bool IsControl(unsigned char *pk, size_t pklen) const;

	/* if normal-regular, tell apart system/user messages */
	bool IsSystem(unsigned char *pk, size_t pklen) const;

	/* for control-regular, return type control packet */
	int IdentifyControl(unsigned char *pk, size_t pklen) const;

	/* for normal-regular (user and system), return msgtype */
	int IdentifyRegular(unsigned char *pk, size_t pklen) const;
};

};

#endif /* LIBTW_PROTO_REGULAR_H */
