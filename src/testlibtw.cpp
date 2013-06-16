/* testlibtw.cpp - (C) 2012, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <vector>
#include <string>
#include <cstdlib>

#include <libtw/master.h>

int
main(int argc, char **argv)
{
	tw::MasterComm m;
	vector<string> v;
	v.push_back("example.org");
	v.push_back("example.org:1337");
	v.push_back("1.23.45.123");
	v.push_back("1.23.45.123:12345");
	v.push_back("[aa:bb:cc::ff]");
	v.push_back("[aa:bb:cc::ff]:5");
	m.SetMasters(v);
	vector<string> r;
	m.GetList(r);
	return EXIT_SUCCESS;
}
