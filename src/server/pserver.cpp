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


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#if !defined(PLATFORM_WINDOWS)
# include <signal.h>
#endif

#include <vector>

#include "Config.h"
#include "Platform.h"
#include "Debug.h"
#include "Logger.h"

#include "Network.h"
#include "SysAccess.h"
#include "ConfigParser.hpp"
#include "game.hpp"

using namespace std;

ConfigParser config;

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
		log_msg("listensock", "socket() failed (%d: %s)", errno, strerror(errno));
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
		log_msg("listensock", "setsockopt:SO_REUSEADDR failed");
		return -4;
	}
	
	socket_setnonblocking(sock);
	
	if (socket_bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		log_msg("listensock", "bind() failed: (%d: %s)", errno, strerror(errno));
		return -2;
	}
	
	if (socket_listen(sock, backlog) == -1)
	{
		log_msg("listensock", "listen() failed: (%d: %s)", errno, strerror(errno));
		return -3;
	}
	
	
	log_msg("listensock", "listening (port: %d)", port);
	
	return sock;
}

int mainloop()
{
	int listenfd;
	if ((listenfd = listensock_create(config.getInt("port"), SERVER_LISTEN_BACKLOG)) < 0)
	{
		log_msg("listensock", "(%d) error creating socket", listenfd);
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
				log_msg("listensock", "(%d) accepted connection (%s)",
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
					log_msg("clientsock", "(%d) socket closed (%d: %s)", sender, errno, strerror(errno));

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

bool config_load()
{
	// include defaults
	#include "server_variables.hpp"
	
	
	// create config-dir if it doesn't yet exist
	sys_mkdir(sys_config_path());
	
	
	char cfgfile[1024];
	snprintf(cfgfile, sizeof(cfgfile), "%s/server.cfg", sys_config_path());
	
	if (config.load(cfgfile))
		log_msg("config", "Loaded configuration from %s", cfgfile);
	else
	{
		if (config.save(cfgfile))
			log_msg("config", "Saved initial configuration to %s", cfgfile);
	}
	
	return true;
}

int main(int argc, char **argv)
{
	log_set(stdout, 0);
	
	log_msg("main", "HoldingNuts pserver (version %d.%d.%d; svn %s)",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION,
		VERSIONSTR_SVN);
	
#if !defined(PLATFORM_WINDOWS)
	// ignore broken-pipe signal eventually caused by sockets
	signal(SIGPIPE, SIG_IGN);
#endif
	
	// init PRNG
	srand((unsigned) time(NULL));
	
	
	// use config-directory set on command-line
	if (argc >= 3 && (argv[1][0] == '-' && argv[1][1] == 'c'))
	{
		const char *path = argv[2];
		
		sys_set_config_path(path);
		log_msg("config", "Using manual config-directory '%s'", path);
	}
	
	
	// load config
	config_load();
	config.print();
	
	
	// start logging
	filetype *fplog = NULL;
	if (config.getBool("log"))
	{
		char logfile[1024];
		snprintf(logfile, sizeof(logfile), "%s/server.log", sys_config_path());
		fplog = file_open(logfile, mode_write);
		
		// log destination
		log_set(stdout, fplog);
		
		// log timestamp
		if (config.getBool("log_timestamp"))
			log_use_timestamp(1);
	}
	
	
	network_init();
	
	mainloop();
	
	network_shutdown();
	
	
	// close log-file
	if (fplog)
		file_close(fplog);
	
	return 0;
}
