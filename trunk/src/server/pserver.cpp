/*
 * Copyright 2008, Dominik Geyer
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
 */


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#if not defined(PLATFORM_WINDOWS)
# include <signal.h>
#endif

#include <vector>

#include "Config.h"
#include "Platform.h"
#include "Debug.h"

#include "Network.h"
#include "game.hpp"

using namespace std;


socktype fdset_get_descriptor(fd_set *fds)
{
        socktype i = 0;

        while(!FD_ISSET(i, fds))
                i++;

        return i;
}

int listensock_create(unsigned int port, int backlog)
{
	int listenfd;
	socktype sock;
	struct sockaddr_in addr;

	if ((listenfd = socket_create(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		dbg_print("listensock", "socket() failed (%d: %s)", errno, strerror(errno));
		return -1;
	}
	
	sock = listenfd;
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	
	/* Use address even it is in use (TIME_WAIT) */
	int reuse = 1;
	if (socket_setopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		dbg_print("listensock", "setsockopt:SO_REUSEADDR failed");
		return -4;
	}
	
	socket_setnonblocking(sock);
	
	if (socket_bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		dbg_print("listensock", "bind() failed: (%d: %s)", errno, strerror(errno));
		return -2;
	}
	
	if (socket_listen(sock, backlog) == -1)
	{
		dbg_print("listensock", "listen() failed: (%d: %s)", errno, strerror(errno));
		return -3;
	}
	
	
	dbg_print("listensock", "listening (port: %d)", port);
	
	return sock;
}

int mainloop()
{
	int listenfd;
	if ((listenfd = listensock_create(DEFAULT_SERVER_PORT, SERVER_LISTEN_BACKLOG)) < 0)
	{
		dbg_print("listensock", "(%d) error creating socket", listenfd);
		return 1;
	}
	
	
	socktype sock = listenfd;
	socktype max;     /* highest socket number select() uses */
	fd_set fds;
	
	
	for (;;)
	{
		// handle game
		gameloop();
		
		struct timeval timeout;  /* timeout for select */
		timeout.tv_sec  = 0;
		timeout.tv_usec = SERVER_SELECT_TIMEOUT_USEC;
		
		FD_ZERO(&fds);
		
		/* add listening socket to the fd-set */
		FD_SET(sock, &fds);
		max = sock;
		
		/* add control clients to select-SET */
		vector<clientcon> &clientvec = get_client_vector();
		for (unsigned int i=0; i < clientvec.size(); i++)
		{
			socktype client_sock = clientvec[i].sock;
			FD_SET(client_sock, &fds);
			
			if (client_sock > max)
				max = client_sock;
		}
		
		
		// are there any modified descriptors?
		if (select(max + 1, &fds, NULL, NULL, &timeout))
		{
			// listen socket
			if (FD_ISSET(sock, &fds))
			{
				sockaddr_in saddr;
				unsigned int saddrlen = sizeof(saddr);
				memset(&saddr, 0, sizeof(sockaddr_in));
				
				socktype client_sock = socket_accept(sock, (struct sockaddr*) &saddr, &saddrlen);
				dbg_print("listensock", "(%d) accepted connection (%s)",
					client_sock, inet_ntoa((struct in_addr) saddr.sin_addr));
				
				socket_setnonblocking(client_sock);
				
				client_add(client_sock, &saddr);
				
				FD_CLR(sock, &fds);
			}
			else
			{
				// handle only one client-request per iteration
				socktype sender = fdset_get_descriptor(&fds);
				
				int status = client_handle(sender);
				if (status <= 0)
				{
					if (!status)
						errno = 0;
					dbg_print("clientsock", "(%d) socket closed (%d: %s)", sender, errno, strerror(errno));

					client_remove(sender);
					
					FD_CLR(sender, &fds);
				}
			}
		}
		else
		{
			// select-timeout occured
			//fprintf(stderr, ".\n"); fflush(stderr);
		}
	}
	
	return 0;
}

int main(int argc, char **argv)
{
	dbg_print("main", "HoldingNuts pserver (version %d.%d.%d)",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
	
#if not defined(PLATFORM_WINDOWS)
	// ignore broken-pipe signal eventually caused by sockets
	signal(SIGPIPE, SIG_IGN);
#endif
	
	// init PRNG
	srand((unsigned) time(NULL));
	
	network_init();
	
	mainloop();
	
	network_shutdown();
	
	return 0;
}
