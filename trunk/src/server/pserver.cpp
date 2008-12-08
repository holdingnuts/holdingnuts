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
#include "Network.h"

#include "Debug.h"


using namespace std;

extern int gameloop();
extern void get_sock_vector(vector<socktype> &vec);
extern bool client_add(socktype sock);
extern bool client_remove(socktype sock);
extern int client_handle(socktype sock);
extern int send_msg(socktype sock, const char *msg);


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
	if ((listenfd = listensock_create(DEFAULT_SERVER_PORT, 3)) < 0)
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
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		
		FD_ZERO(&fds);
		
		/* add listening socket to the fd-set */
		FD_SET(sock, &fds);
		max = sock;
		
		/* add control clients to select-SET */
		vector<socktype> sockvec;
		get_sock_vector(sockvec);
		
		for (unsigned i=0; i < sockvec.size(); i++)
		{
			socktype client_sock = sockvec[i];
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
				
				// TODO: handle server-full
				client_add(client_sock);
				
				FD_CLR(sock, &fds);
				
				// send initial data
				char msg[1024];
				snprintf(msg, sizeof(msg), "PSERVER %d %d",
					VERSION_CREATE(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION),
					client_sock);
				send_msg(client_sock, msg);
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
	dbg_print("main", "server version %d.%d.%d",
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
