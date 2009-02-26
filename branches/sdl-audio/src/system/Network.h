/*
 * Copyright 2008, 2009, Dominik Geyer
 *
 * This file is part of HoldingNuts.
 *
 * HoldingNuts is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoldingNuts is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoldingNuts.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


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
int socket_connect(socktype sockfd, const struct sockaddr *addr, unsigned int addrlen);
int socket_close(socktype fd);

int socket_read(socktype fd, void *buf, size_t count);
int socket_write(socktype fd, const void *buf, size_t count);

int socket_setopt(socktype s, int level, int optname, const void *optval, int optlen);
int socket_setnonblocking(socktype sock);

int network_isinprogress();

int network_init();
int network_shutdown();

#if defined __cplusplus
    }
#endif

#endif /* _NETWORK_H */
