/* testlibtw.cpp - (C) 2012, Timo Buhrmester
 * libtw - uhm
  * See README for contact-, COPYING for license information.  */

#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <libtw/master.h>
#include <libtw/info.h>

int
main(int argc, char **argv)
{
	vector<string> masters;

	//masters.push_back("master1.teeworlds.com");
	masters.push_back("master2.teeworlds.com");
	masters.push_back("master3.teeworlds.com");
	//masters.push_back("master4.teeworlds.com");

	tw::MasterComm m;
	m.SetMasters(masters);

	vector<string> result;
	m.GetList(result);

	fprintf(stderr, "got %zu servers:\n", result.size());
	/*for(vector<string>::const_iterator it = result.begin();
			it != result.end(); it++)
		fprintf(stderr, "\t'%s'\n", it->c_str());*/


	tw::InfoComm i;
	i.SetServers(result);
	int suc = i.Refresh();

	fprintf(stderr, "refreshed %d servers (of %zu)\n",
			suc, result.size());
	fprintf(stderr, "bye\n");

	return EXIT_SUCCESS;
}
