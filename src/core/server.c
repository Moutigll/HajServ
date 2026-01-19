#include <string.h>

#include "../../include/core/server.h"
#include "../../include/logger.h"

int serverInit(s_server *server, uint16_t port, s_socketOpts opts)
{
	if (!server)
		return -1;

	memset(server, 0, sizeof(*server));
	server->port = port;
	server->opts = opts;

	int count = serverCreateSockets(port, opts, server->sockets);
	if (count < 0)
		return -1;

	server->socketCount = count;
	LOG(LOG_INFO, "Server initialized on port %u with %d sockets", port, count);
	return 0;
}