#include <arpa/inet.h>
#include <errno.h> // IWYU pragma: keep
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../../../include/core/sockets/create.h"
#include "../../../include/logger.h"

static int applySocketOpts(int fd, const s_socketOpts opts)
{
	int val = 1;

	if (opts.reuseAddr)
	{
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
		{
			LOG(LOG_ERROR, "Failed to set SO_REUSEADDR on fd %d: %s", fd, strerror(errno));
			return (-1);
		}
	}

#ifdef SO_REUSEPORT
	if (opts.reusePort)
	{
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) < 0)
		{
			LOG(LOG_ERROR, "Failed to set SO_REUSEPORT on fd %d: %s", fd, strerror(errno));
			return (-1);
		}
	}
#endif

#ifdef SO_BINDTODEVICE
	if (opts.bindInterface)
	{
		if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
					   opts.bindInterface,
					   strlen(opts.bindInterface)) < 0)
		{
			LOG(LOG_ERROR, "Failed to set SO_BINDTODEVICE on fd %d: %s", fd, strerror(errno));
			return (-1);
		}
	}
#endif

	return (0);
}

static int bindIp(int				fd,
				  e_socketFamily	family,
				  uint16_t			port,
				  const char		*ipBind)
{
	if (family == SOCKET_IPV4)
	{
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		if (ipBind)
		{
			if (inet_pton(AF_INET, ipBind, &addr.sin_addr) <= 0)
			{
				LOG(LOG_ERROR, "Invalid IPv4 address: %s", ipBind);
				return (-1);
			}
		}
		else
			addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			LOG(LOG_ERROR, "Failed to bind IPv4 address on fd %d: %s", fd, strerror(errno));
			return (-1);
		}
	}
	else // SOCKET_IPV6
	{
		struct sockaddr_in6 addr = {0};
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(port);
		if (ipBind)
		{
			if (inet_pton(AF_INET6, ipBind, &addr.sin6_addr) <= 0)
			{
				LOG(LOG_ERROR, "Invalid IPv6 address: %s", ipBind);
				return (-1);
			}
		}
		else
			addr.sin6_addr = in6addr_any;

		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			LOG(LOG_ERROR, "Failed to bind IPv6 address on fd %d: %s", fd, strerror(errno));
			return (-1);
		}
	}

	return (0);
}

static int createSingleSocket(uint16_t				port,
							  e_socketFamily		family,
							  e_socketType			type,
							  const s_socketOpts	opts)
{
	int fd = socket((family == SOCKET_IPV4 ? AF_INET : AF_INET6),
					(type == SOCKET_TCP ? SOCK_STREAM : SOCK_DGRAM),
					0);
	if (fd < 0)
	{
		LOG(LOG_ERROR, "Failed to create socket %s/%s for port %d: %s",
			(family == SOCKET_IPV4 ? "IPv4" : "IPv6"),
			(type == SOCKET_TCP ? "TCP" : "UDP"),
			port, strerror(errno));
		return (-1);
	}

	if (applySocketOpts(fd, opts) < 0)
		goto error;

	const char *ipBind = (family == SOCKET_IPV4) ? opts.bindIpV4 : opts.bindIpV6;
	if (bindIp(fd, family, port, ipBind) < 0)
		goto error;

	if (type == SOCKET_TCP && listen(fd, opts.backlog) < 0)
	{
		LOG(LOG_ERROR, "Failed to listen on socket fd %d: %s", fd, strerror(errno));
		goto error;
	}

	LOG(LOG_INFO, "Created %s/%s socket on port %d (fd %d)",
		(family == SOCKET_IPV4 ? "IPv4" : "IPv6"),
		(type == SOCKET_TCP ? "TCP" : "UDP"),
		port, fd);
	return (fd);

error:
	close(fd);
	return (-1);
}

int serverCreateSockets(uint16_t			port,
						const s_socketOpts	opts,
						s_listenSocket		sockets[SERV_MAX_SOCKETS])
{
	int count = 0;
	int fd;

	/* Initialize sockets to -1 */
	for (int i = 0; i < SERV_MAX_SOCKETS; i++)
		sockets[i].fd = -1;

	/* Define all possible socket combinations */
	struct {
		e_socketFamily family;
		e_socketType   type;
		int			enabled;
	} combos[] = {
		{ SOCKET_IPV4, SOCKET_TCP, opts.enableIpv4 && opts.enableTcp },
		{ SOCKET_IPV6, SOCKET_TCP, opts.enableIpv6 && opts.enableTcp },
		{ SOCKET_IPV4, SOCKET_UDP, opts.enableIpv4 && opts.enableUdp },
		{ SOCKET_IPV6, SOCKET_UDP, opts.enableIpv6 && opts.enableUdp }
	};

	for (int i = 0; i < 4; i++)
	{
		if (!combos[i].enabled)
			continue;

		fd = createSingleSocket(port, combos[i].family, combos[i].type, opts);
		if (fd < 0)
		{
			for (int i = 0; i < count; i++)
				if (sockets[i].fd >= 0)
					close(sockets[i].fd);

			return (-1);
		}

		sockets[count++] = (s_listenSocket){ fd, combos[i].type, combos[i].family };
	}

	return (count);
}
