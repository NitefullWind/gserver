#include <tinyserver/logger.h>
#include <sys/resource.h>

#include "GServer.h"

using namespace tinyserver;
using namespace gserver;

#define CORE_SIZE   1024 * 1024 * 500

int main(int argc, char **argv)
{
    struct rlimit rlmt;
    rlmt.rlim_cur = (rlim_t)CORE_SIZE;
    rlmt.rlim_max  = (rlim_t)CORE_SIZE;
	if (setrlimit(RLIMIT_CORE, &rlmt) == -1) {
		return -1; 
	}

	Logger::SetLevel(Logger::Debug);
	
	gserver::GServer gserver(8086);
	gserver.start();

	return 0;
}