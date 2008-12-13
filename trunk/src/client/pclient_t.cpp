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
#include <string>

#include "Config.h"
#include "Platform.h"
#include "Network.h"

#include "Debug.h"


using namespace std;

char server_address[256];

#if 0
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


// returns zero if no cmd was found or no bytes remaining after exec
int server_parsebuffer(clientcon *client)
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
		
		dbg_print("clientsock", "(%d) command: '%s' (len=%d)", client->sock, cmd, found_nl);
		//server_execute(client, cmd);
		
		// move the rest to front
		memmove(client->msgbuf, client->msgbuf + found_nl + 1, client->buflen - (found_nl + 1));
		client->buflen -= found_nl + 1;
		//dbg_print("clientsock", "(%d) new buffer after cmd (bufferlen=%d)", client->sock, client->buflen);
		
		return client->buflen;
	}
	else
		return 0;
}
#endif

int server_handle(socktype sock)
{
	char buf[1024];
	int bytes;
	
	// return early on client close/error
	if ((bytes = socket_read(sock, buf, sizeof(buf))) <= 0)
		return bytes;
	
	
	dbg_print("connectsock", "(%d) DATA len=%d", sock, bytes);
	
#if 0
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
#endif
	
	return bytes;
}

int connectsock_create(const char *server, unsigned int port)
{
	int connectfd;
	socktype sock;
	struct sockaddr_in addr;

	if ((connectfd = socket_create(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		dbg_print("connectsock", "socket() failed (%d: %s)", errno, strerror(errno));
		return -1;
	}
	
	sock = connectfd;
	
	struct hostent *host_entry;
	host_entry = gethostbyname(server);
	
	if(!host_entry)
	{
		dbg_print("connectsock", "gethostbyname() failed (%d: %s)", errno, strerror(errno));
		return -2;
	}
	
	memcpy(&addr.sin_addr.s_addr, host_entry->h_addr_list[0], host_entry->h_length);
	addr.sin_family = host_entry->h_addrtype;
	addr.sin_port = htons(port);
	
	socket_setnonblocking(sock);
	
	// FIXME: check for immediate error
	socket_connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	
#if 0
	time_t timeout_start = time(NULL);
	
	int status;
	
	do
	{
		status = socket_connect(sock, (struct sockaddr*)&addr, sizeof(addr));
		
		if (status >= 0)
			break;
	}
	while ((int)difftime(time(NULL), timeout_start) < CLIENT_CONNECT_TIMEOUT);
	
	if (status < 0)
		return status;
#endif
	return sock;
}

int client_execute(const char *cmd)
{
	dbg_print("client_execute", "len=%d text=%s", (int)strlen(cmd), cmd);
	
	return 0;
}

#if defined (PLATFORM_WINDOWS)
// non-blocking windows console input handling
int input_handle()
{
	static char inpBuf[1024] = "";
	static unsigned int inpBufLen = 0;
	
	//http://api.farmanager.com/en/winapi/readconsoleinput.html
	//http://api.farmanager.com/en/winapi/input_record.html
	//http://api.farmanager.com/en/winapi/key_event_record.html
	
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (WaitForSingleObject(hStdin, 10) != WAIT_TIMEOUT)
	{
		
		char buf;
		INPUT_RECORD inr;
		DWORD numRead;
		ReadConsoleInput(hStdin, &inr, 1, &numRead);
		
		if (inr.EventType == KEY_EVENT && inr.Event.KeyEvent.bKeyDown)
		{
			buf = inr.Event.KeyEvent.uChar.AsciiChar;
			//dbg_print("windows", "got input: %c 0x%x", buf, buf);
			
			if (buf == '\r')
			{
				inpBuf[inpBufLen] = '\0';
				fprintf(stdout, "\r\n"); fflush(stdout);
				
				client_execute(inpBuf);
				
				inpBufLen = 0;
			}
			else
			{
				if (buf)
				{
					fprintf(stdout, "%c", buf); fflush(stdout);
					inpBuf[inpBufLen++] = buf;
				}
			}
		}
	}
	
	return 0;
}
#else
int input_handle()
{
	const unsigned int max_input = 1024;
	char input[max_input];
	
	fgets(input, max_input, stdin);
	input[strlen(input)-1] = '\0';
	
	//dbg_print("input", "len=%d text=%s", (int)strlen(input), input);
	client_execute(input);
	
	return 0;
}
#endif

int gameloop()
{
	
	
	return 0;
}

int mainloop()
{
	socktype sock;
	socktype max;     /* highest socket number select() uses */
	
	fd_set fds;
	
	int connectfd;
	connectfd = connectsock_create(server_address, DEFAULT_SERVER_PORT);
	
	sock = connectfd;
	
	// check for connect
	{
		struct timeval timeout;  /* timeout for select */
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		
		fd_set fds_write, fds_err;
		
		FD_ZERO(&fds_write);
		FD_ZERO(&fds_err);
		
		FD_SET(sock, &fds_write);
		FD_SET(sock, &fds_err);
		
		if (select(sock + 1, NULL, &fds_write, &fds_err, &timeout))
		{
			if (FD_ISSET(sock, &fds_err))
			{
				dbg_print("connectsock", "error connecting socket");
				return -1;
			}
			
			if (FD_ISSET(sock, &fds_write))
			{
				dbg_print("connectsock", "connected");
			}
		}
	}
	
	for (;;)
	{
		// handle game
		gameloop();
		
		struct timeval timeout;  /* timeout for select */
		timeout.tv_sec = 0;
		timeout.tv_usec = 50000;
		
		FD_ZERO(&fds);
		
		/* add stdin and connected-socket to the fd-set */
#if not defined(PLATFORM_WINDOWS)
		FD_SET(STDIN_FILENO, &fds);
#else
		input_handle();
#endif
		FD_SET(sock, &fds);
		max = sock;
		
		// are there any modified descriptors?
		if (select(max + 1, &fds, NULL, NULL, &timeout))
		{
#if not defined(PLATFORM_WINDOWS)
			if (FD_ISSET(STDIN_FILENO, &fds))
				input_handle();
#endif
			
			if (FD_ISSET(sock, &fds))
			{
				int status = server_handle(sock);
				if (status <= 0)
				{
					dbg_print("connectsock", "(%d) closed connection (%d: %s)", sock, errno, strerror(errno));
					socket_close(sock);
					
					return -1;
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
	dbg_print("main", "HoldingNuts pclient_t version %d.%d.%d",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
	
#if not defined(PLATFORM_WINDOWS)
	// ignore broken-pipe signal eventually caused by sockets
	signal(SIGPIPE, SIG_IGN);
#endif

#if defined(PLATFORM_WINDOWS)
	//SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT);
#endif
	
	network_init();
	
	// first arg is the hostname
	if (argc < 2)
		strcpy(server_address, "localhost");
	else
		snprintf(server_address, sizeof(server_address), "%s", argv[1]);
	
	mainloop();
	
	network_shutdown();
	
	return 0;
}
