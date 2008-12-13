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

#include "Tokenizer.hpp"
#include "Debug.h"


using namespace std;

typedef struct {
	socktype sock;
	
	char msgbuf[1024];
	int buflen;
} servercon;

servercon srv;

char server_address[256];
int my_cid, my_gid, my_tid;

int send_msg(const char *msg)
{
	const int bufsize = 1024;
	char buf[bufsize];
	int len = snprintf(buf, bufsize, "%s\n", msg);
	int bytes = socket_write(srv.sock, buf, len);
	
	// FIXME: send remaining bytes if not all have been sent
	
	if (len != bytes)
		dbg_print("connectsock", "(%d) warning: not all bytes written (%d != %d)", srv.sock, len, bytes);
	
	return bytes;
}

int server_execute(const char *cmd)
{
	char msg[1024];
	
	Tokenizer t;
	t.parse(cmd);  // parse the command line
	
	if (!t.getCount())
		return 0;
	
	//dbg_print("server", "executing '%s'", cmd);
	
	// get first arg
	string command;
	t.getNext(command);
	
	if (command == "PSERVER")
	{
		unsigned int version = string2int(t.getNext());
		my_cid = string2int(t.getNext());
		
		dbg_print("server", "Server running version %d.%d.%d. Your client ID is %d",
			VERSION_GETMAJOR(version), VERSION_GETMINOR(version), VERSION_GETREVISION(version),
			my_cid);
		
		snprintf(msg, sizeof(msg), "PCLIENT %d",
			VERSION_CREATE(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION));
		
		send_msg(msg);
		
		// TODO: send user info
	}
	else if (command == "OK")
	{
		
	}
	else if (command == "ERR")
	{
		dbg_print("server", "last cmd error!");
	}
	else if (command == "GAMELIST")
	{
		dbg_print("server", "list of games: %s", cmd);
	}
	else if (command == "MSG")
	{
		string chatmsg;
		for (unsigned int i=3; i < t.getCount(); i++)
			chatmsg += t[i] + ' ';
		
		dbg_print("chat", "[%s]: %s", t[2].c_str(), chatmsg.c_str());
	}
	else
	{
		dbg_print("server", "unknown cmd: %s", cmd);
	}
	
	
	return 0;
}

// returns zero if no cmd was found or no bytes remaining after exec
int server_parsebuffer()
{
	//dbg_print("clientsock", "(%d) parse (bufferlen=%d)", srv.sock, srv.buflen);
	
	int found_nl = -1;
	for (int i=0; i < srv.buflen; i++)
	{
		if (srv.msgbuf[i] == '\r')
			srv.msgbuf[i] = ' ';  // space won't hurt
		else if (srv.msgbuf[i] == '\n')
		{
			found_nl = i;
			break;
		}
	}
	
	int retval = 0;
	
	// is there a command in queue?
	if (found_nl != -1)
	{
		// extract command
		char cmd[sizeof(srv.msgbuf)];
		memcpy(cmd, srv.msgbuf, found_nl);
		cmd[found_nl] = '\0';
		
		//dbg_print("clientsock", "(%d) command: '%s' (len=%d)", srv.sock, cmd, found_nl);
		if (server_execute(cmd) != -1)
		{
			// move the rest to front
			memmove(srv.msgbuf, srv.msgbuf + found_nl + 1, srv.buflen - (found_nl + 1));
			srv.buflen -= found_nl + 1;
			//dbg_print("clientsock", "(%d) new buffer after cmd (bufferlen=%d)", srv.sock, srv.buflen);
			
			retval = srv.buflen;
		}
		else
			retval = 0;
	}
	else
		retval = 0;
	
	return retval;
}

int server_handle()
{
	socktype sock = srv.sock;
	char buf[1024];
	int bytes;
	
	// return early on client close/error
	if ((bytes = socket_read(sock, buf, sizeof(buf))) <= 0)
		return bytes;
	
	
	//dbg_print("connectsock", "(%d) DATA len=%d", sock, bytes);
	
	if (srv.buflen + bytes > (int)sizeof(srv.msgbuf))
	{
		dbg_print("clientsock", "(%d) error: buffer size exceeded", sock);
		srv.buflen = 0;
	}
	else
	{
		memcpy(srv.msgbuf + srv.buflen, buf, bytes);
		srv.buflen += bytes;
		
		// parse and execute all commands in queue
		while (server_parsebuffer());
	}
	
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
	
	socket_connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	
	// FIXME: check for immediate error
	//if (status == -1 && (errno != EINPROGRESS || errno != EWOULDBLOCK))
	
	return sock;
}

int client_execute(const char *cmd)
{
	char msg[1024];
	
	Tokenizer t;
	t.parse(cmd);  // parse the command line
	
	if (!t.getCount())
		return 0;
	
	//dbg_print("input", "executing '%s'", cmd);
	
	// get first arg
	string command;
	t.getNext(command);
	
	if (command == "register")
	{
		int gid = string2int(t.getNext());
		
		my_gid = gid;  // FIXME
		
		snprintf(msg, sizeof(msg), "REGISTER %d", gid);
		send_msg(msg);
	}
	else if (command == "gamelist")
	{
		send_msg("REQUEST gamelist");
	}
	else if (command == "reset" || command == "fold" || command == "check" ||
		 command == "call" || command == "allin")
	{
		snprintf(msg, sizeof(msg), "ACTION %d %s", my_gid, command.c_str());
		send_msg(msg);
	}
	else if (command == "bet" || command == "raise")
	{
		snprintf(msg, sizeof(msg), "ACTION %d %s %s",
			my_gid, command.c_str(), t[1].c_str());
		send_msg(msg);
	}
	else
		dbg_print("input", "unknown cmd: %s", cmd);
	
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
					if (buf == '\b')
					{
						if (inpBufLen)
						{
							inpBufLen--;
							fprintf(stdout, "\b \b"); fflush(stdout);
						}
					}
					else
					{
						// prevent buffer-overflow
						if (inpBufLen == sizeof(inpBuf) -1)
							inpBufLen = 0;
						
						fprintf(stdout, "%c", buf); fflush(stdout);
						inpBuf[inpBufLen++] = buf;
					}
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
	
	
	memset(&srv, 0, sizeof(srv));
	srv.sock = sock;
	
	
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
				int status = server_handle();
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
