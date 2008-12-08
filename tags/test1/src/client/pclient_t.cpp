#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vector>
#include <string>

#include "Config.h"
#include "Platform.h"
#include "Network.h"

#include "Debug.h"


using namespace std;

typedef enum {
	Collecting=1,
	Playing
} gamestate;

typedef enum {
	Settings=0x1,
	Foyer=0x2,
	Table=0x4
} snaptype;


socktype fdset_get_descriptor(fd_set *fds)
{
        socktype i = 0;

        while(!FD_ISSET(i, fds))
                i++;

        return i;
}

int send_msg(socktype sock, const char *msg)
{
	const int bufsize = 1024;
	char buf[bufsize];
	int len = snprintf(buf, bufsize, "%s\n", msg);
	int bytes = socket_write(sock, buf, len);
	
	// FIXME: send remaining bytes if not all have been sent
	
	if (len != bytes)
		dbg_print("clientsock", "(%d) warning: not all bytes written (%d != %d)", sock, len, bytes);
	
	return bytes;
}


void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
{
	// Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos     = str.find_first_of(delimiters, lastPos);
	
	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

// returns zero if no cmd was found or no bytes remaining after exec
int client_parsebuffer(clientcon *client)
{
	//dbg_print("clientsock", "(%d) parse (bufferlen=%d)", client->sock, client->buflen);
	
	int found_nl = -1;
	for (int i=0; i < client->buflen; i++)
	{
		if (client->msgbuf[i] == '\r')
			client->msgbuf[i] = ' ';  // debugging purpose
		else if (client->msgbuf[i] == '\n')
		{
			found_nl = i;
			break;
		}
	}
	
	// is there a command in queue?
	if (found_nl != -1)
	{
		// extract command
		char cmd[sizeof(client->msgbuf)];
		memcpy(cmd, client->msgbuf, found_nl);
		cmd[found_nl] = '\0';
		
		//dbg_print("clientsock", "(%d) command: '%s' (len=%d)", client->sock, cmd, found_nl);
		client_execute(client, cmd);
		
		// move the rest to front
		memmove(client->msgbuf, client->msgbuf + found_nl + 1, client->buflen - (found_nl + 1));
		client->buflen -= found_nl + 1;
		//dbg_print("clientsock", "(%d) new buffer after cmd (bufferlen=%d)", client->sock, client->buflen);
		
		return client->buflen;
	}
	else
		return 0;
}

int client_handle(socktype sock)
{
	char buf[1024];
	int bytes;
	
	errno = 0;   // workaround
	
	// return early on client close/error
	if ((bytes = socket_read(sock, buf, sizeof(buf))) <= 0)
		return bytes;
	
	
	//dbg_print("clientsock", "(%d) DATA len=%d", sock, bytes);
	
	clientcon *client = get_client_by_sock(sock);
	if (!client)
	{
		dbg_print("clientsock", "(%d) error: no client associated", sock);
		return -1;
	}
	
	if (client->buflen + bytes > (int)sizeof(client->msgbuf))
	{
		dbg_print("clientsock", "(%d) error: buffer size exceeded", sock);
		client->buflen = 0;
	}
	else
	{
		memcpy(client->msgbuf + client->buflen, buf, bytes);
		client->buflen += bytes;
		
		// parse and execute all commands in queue
		while (client_parsebuffer(client));
	}
	
	return bytes;
}


int gameloop()
{
	if (snap_update)
	{
		if (snap_update & Foyer)
		{
			update_foyer_snapshot();
			send_snapshot(Foyer);
		}
		
		snap_update = 0x0;
	}
	
	return 0;
}

int mainloop()
{
	socktype sock;
	socktype max;     /* highest socket number select() uses */
	
	fd_set fds;
	
	int listenfd;
	if ((listenfd = listensock_create(DEFAULT_SERVER_PORT, 3)) < 0)
	{
		dbg_print("listensock", "(%d) error creating socket", sock);
		return 1;
	}
	
	sock = listenfd;
	
	for (;;)
	{
		struct timeval timeout;  /* timeout for select */
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		
		FD_ZERO(&fds);
		
		/* add listening socket to the fd-set */
		FD_SET(sock, &fds);
		max = sock;
		
		/* add control clients to select-SET */
		for (vector<clientcon>::iterator e = clients.begin(); e != clients.end(); e++)
		{
			socktype client_sock = e->sock;
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
				socktype client_sock = socket_accept(sock, NULL, 0);
				dbg_print("listensock", "(%d) accepted connection", client_sock);
				
				socket_setnonblocking(client_sock);
				
				// TODO: handle server-full
				client_add(client_sock);
				snap_update |= Foyer;
				
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
					dbg_print("clientsock", "(%d) closed connection (%d: %s)", sender, errno, strerror(errno));
					socket_close(sender);
					
					// remove client from vector
					client_remove(sender);
					snap_update |= Foyer;
					
					FD_CLR(sender, &fds);
				}
			}
		}
		else
		{
			// select-timeout occured
			//fprintf(stderr, ".\n"); fflush(stderr);
		}
		
		// handle game
		gameloop();
	}
	
	return 0;
}

int main(int argc, char **argv)
{
	dbg_print("main", "client version %d.%d.%d",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
	
#if not defined(PLATFORM_WINDOWS)
	// ignore broken-pipe signal eventually caused by sockets
	signal(SIGPIPE, SIG_IGN);
#endif
	
	network_init();
	
	mainloop();
	
	network_shutdown();
	
	return 0;
}
