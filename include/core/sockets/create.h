#ifndef CORE_SOCKETS_H
# define CORE_SOCKETS_H

#include <stdint.h>


# define SERV_MAX_SOCKETS 4


typedef enum e_socketType
{
	SOCKET_TCP,
	SOCKET_UDP
} e_socketType;

typedef enum e_socketFamily
{
	SOCKET_IPV4,
	SOCKET_IPV6
} e_socketFamily;


typedef struct s_socketOpts
{
	int		backlog;
	int		reuseAddr;
	int		reusePort;
	int		enableIpv4;
	int		enableIpv6;
	int		enableTcp;
	int		enableUdp;
	char	*bindIpV4;
	char	*bindIpV6;
	char	*bindInterface;
} s_socketOpts;

#define SOCKET_OPTS_DEFAULT { 128, 1, 0, 1, 0, 1, 0, NULL, NULL, NULL }


typedef struct s_listenSocket
{
	int				fd;
	e_socketType	type;
	e_socketFamily	family;
} s_listenSocket;


int serverCreateSockets(uint16_t port, s_socketOpts opts, s_listenSocket sockets[SERV_MAX_SOCKETS]);


#endif /* CORE_SOCKETS_H */