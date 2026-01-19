#ifndef CORE_SERVER_H
# define CORE_SERVER_H

#include "sockets/create.h"
#include <stddef.h>

typedef struct s_server
{
	uint16_t		port;
	s_socketOpts	opts;

	int				socketCount;
	s_listenSocket	sockets[SERV_MAX_SOCKETS];
} s_server;

int serverInit(s_server *server, uint16_t port, s_socketOpts opts);

#endif /* CORE_SERVER_H */