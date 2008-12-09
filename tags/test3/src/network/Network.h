#ifndef _NETWORK_H
#define _NETWORK_H

#include "Platform.h"

#if defined(PLATFORM_WINDOWS)
# include <winsock2.h>
#else
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <netdb.h>
# include <sys/param.h>
# include <errno.h>
# include <fcntl.h>
#include <arpa/inet.h>
#endif

#if defined __cplusplus
        extern "C" {
#endif

#if defined(PLATFORM_WINDOWS)
typedef SOCKET socktype;
#else
typedef int socktype;
#endif

int socket_create(int domain, int type, int protocol);
int socket_bind(socktype sockfd, const struct sockaddr *addr, unsigned int addrlen);
int socket_listen(socktype sockfd, int backlog);
int socket_accept(socktype sockfd, struct sockaddr *addr, unsigned int *addrlen);
int socket_close(socktype fd);

int socket_read(socktype fd, void *buf, size_t count);
int socket_write(socktype fd, const void *buf, size_t count);

int socket_setopt(socktype s, int level, int optname, const void *optval, int optlen);
int socket_setnonblocking(socktype sock);

int network_init();
int network_shutdown();

#if defined __cplusplus
    }
#endif

#endif /* _NETWORK_H */
