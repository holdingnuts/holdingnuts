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


#include "Network.h"

/*
Windows <-> Linux differences:  (http://www.andreadrian.de/select/#mozTocId243918)
- header file is winsock2.h
- WSAGetLastError() instead of errno for error-codes (strerror() doesn't work with these codes)
- setsockopt() with SO_REUSEADDR allows multiple applications to access same port
- setsockopt() uses (char*) as 4th argument instead of (void*); same on Solaris
- socket descriptors are of type SOCKET (u_int) instead of int
- multiple consecutive calls to accept() may return non-consecutive descriptors
- type int instead of socklen_t
- there are no read()/write(); use of functions recv()/send() instead
- closesocket() instead of close()
- winsock2 does not support signals. There's no select() return with WSAEINTR on signals.
- winsock needs to be initialized with WSAStartup() and shutdown with WSACleanup()
*/

int socket_create(int domain, int type, int protocol)
{
	return socket(domain, type, protocol);
}

int socket_bind(socktype sockfd, const struct sockaddr *addr, unsigned int addrlen)
{
	return bind(sockfd, addr, addrlen);
}

int socket_listen(socktype sockfd, int backlog)
{
	return listen(sockfd, backlog);
}

int socket_accept(socktype sockfd, struct sockaddr *addr, unsigned int *addrlen)
{
#if defined(PLATFORM_WINDOWS)
	return accept(sockfd, addr, (int*) addrlen);
#else
	return accept(sockfd, addr, (socklen_t*) addrlen);
#endif
}

int socket_connect(socktype sockfd, const struct sockaddr *addr, unsigned int addrlen)
{
	return connect(sockfd, addr, addrlen);
}

int socket_close(socktype fd)
{
#if defined(PLATFORM_WINDOWS)
	return closesocket(fd);
#else
	return close(fd);
#endif
}

int socket_read(socktype fd, void *buf, size_t count)
{
#if defined(PLATFORM_WINDOWS)
	return recv(fd, (char*)buf, count, 0);
#else
	return read(fd, buf, count);
#endif
}

int socket_write(socktype fd, const void *buf, size_t count)
{
#if defined(PLATFORM_WINDOWS)
	return send(fd, (char*)buf, count, 0);
#else
	return write(fd, buf, count);
#endif
}

int socket_setopt(socktype s, int level, int optname, const void *optval, int optlen)
{
#if defined(PLATFORM_WINDOWS)
	return setsockopt(s, level, optname, (char*) optval, optlen);
#else
	return setsockopt(s, level, optname, optval, optlen);
#endif
}

int socket_setnonblocking(socktype sock)
{
#if defined(PLATFORM_WINDOWS)
	ULONG NonBlock = 1;
	
	if (ioctlsocket(sock, FIONBIO, &NonBlock) == SOCKET_ERROR)
		return -1;
#else
	int opts;
	
	if ((opts = fcntl(sock, F_GETFL)) < 0)
		return -1;
	
	opts |= O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0)
		return -2;
#endif
	return 0;
}

int network_isinprogress()
{
#if defined(PLATFORM_WINDOWS)
	int wserrno = WSAGetLastError();
	
	return (wserrno == WSAEWOULDBLOCK || wserrno == WSAEINPROGRESS);
#else
	return (errno == EWOULDBLOCK || errno == EINPROGRESS);
#endif
}

int network_init()
{
#if defined(PLATFORM_WINDOWS)
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
	return 0;
#endif
}

int network_shutdown()
{
#if defined(PLATFORM_WINDOWS)
	return WSACleanup();
#else
	return 0;
#endif
}
