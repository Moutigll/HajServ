#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../../include/core/server.h"
#include "../../include/logger.h"

int main(void)
{
	s_server srv;
	s_socketOpts opts = SOCKET_OPTS_DEFAULT;

	if (serverInit(&srv, 8080, opts) != 0)
		return 1;

	LOG(LOG_INFO, "Server running on port %u with %d sockets", srv.port, srv.socketCount);

	while (1)
	{
		for (int i = 0; i < srv.socketCount; i++)
		{
			if (srv.sockets[i].type != SOCKET_TCP)
				continue;

			int client_fd = accept(srv.sockets[i].fd, NULL, NULL);
			if (client_fd < 0)
				continue;

			const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World";
			send(client_fd, response, strlen(response), 0);
			close(client_fd);
		}
	}

	// Close all server sockets
	for (int i = 0; i < srv.socketCount; i++)
		if (srv.sockets[i].fd >= 0)
			close(srv.sockets[i].fd);

	return 0;
}
