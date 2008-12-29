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


#include <cstdlib>
#include <cstdio>
#include <string>

#if not defined(PLATFORM_WINDOWS)
# include <signal.h>
#endif

#include "Config.h"
#include "Debug.h"
#include "Network.h"
#include "Tokenizer.hpp"

#include "pclient.hpp"

using namespace std;

struct servercon {
	socktype sock;
	
	char msgbuf[1024];
	int buflen;
	
	int cid;   // our client-id assigned by server
} srv;


//////////////


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
	QString log(cmd);
	((PClient*)qApp)->wMain->addLog(log);
	
	// get first arg
	string command;
	t.getNext(command);
	
	// FIXME: state; check if this is really a pserver
	if (command == "PSERVER")
	{
		unsigned int version = t.getNextInt();
		srv.cid = t.getNextInt();
		
		dbg_print("server", "Server running version %d.%d.%d. Your client ID is %d",
			VERSION_GETMAJOR(version), VERSION_GETMINOR(version), VERSION_GETREVISION(version),
			srv.cid);
		
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
	else if (command == "MSG")
	{
		/*int from =*/ t.getNextInt();
		string sfrom = t.getNext();
		
		string chatmsg = t.getTillEnd();
		
		QString qsfrom(QString::fromStdString(sfrom));
		QString qchatmsg(QString::fromStdString(chatmsg));
		
		((PClient*)qApp)->wMain->addChat(qsfrom, qchatmsg);
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
		return -2;
	}
	
	sock = connectfd;
	
	struct hostent *host_entry;
	host_entry = gethostbyname(server);
	
	if(!host_entry)
	{
		dbg_print("connectsock", "gethostbyname() for %s failed (%d: %s)", server, errno, strerror(errno));
		return -3;
	}
	
	memcpy(&addr.sin_addr.s_addr, host_entry->h_addr_list[0], host_entry->h_length);
	addr.sin_family = host_entry->h_addrtype;
	addr.sin_port = htons(port);
	
	socket_setnonblocking(sock);
	
	int status = socket_connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	
	// check for immediate error
	if (status < 0 && !network_isinprogress())
		return status;
	
	return sock;
}

PClient::PClient(int &argc, char **argv) : QApplication(argc, argv)
{
	connected = false;
	connecting = false;
	
	wMain = new WMain();
	wMain->updateConnectionStatus();
	wMain->show();
}

bool PClient::doConnect(QString strServer, unsigned int port)
{
	wMain->addLog(tr("Connecting..."));
	connectfd = connectsock_create(strServer.toStdString().c_str(), port);
	
	if (connectfd < 0)
		return false;
	
	snw = new QSocketNotifier(connectfd, QSocketNotifier::Write);
	connect(snw, SIGNAL(activated(int)), this, SLOT(slotConnected(int)));
	
	connect_timer = new QTimer(this);
	connect(connect_timer, SIGNAL(timeout()), this, SLOT(slotConnectTimeout()));
	connect_timer->setSingleShot(true);
	connect_timer->start(CLIENT_CONNECT_TIMEOUT * 1000);
	
	connecting = true;
	
	wMain->updateConnectionStatus();
	
	return true;
}

void PClient::doClose()
{
	if (connecting)
	{
		delete snw;
		delete connect_timer;
		connecting = false;
	}
	else
	{
		delete snr;
		send_msg("QUIT");
		connected = false;
	}
	
	socket_close(connectfd);
	
	wMain->addLog(tr("Connection closed."));
	wMain->updateConnectionStatus();
}

void PClient::slotConnectTimeout()
{
	wMain->addLog(tr("Connection timeout."));
	
	delete snw;
	delete connect_timer;
	
	connected = false;
	connecting = false;
	
	wMain->updateConnectionStatus();
}

void PClient::slotConnected(int n)
{
	wMain->addLog(tr("Connected."));
	
	delete snw;
	delete connect_timer;
	
	memset(&srv, 0, sizeof(srv));
	srv.sock = connectfd;
	
	connected = true;
	connecting = false;
	
	snr = new QSocketNotifier(connectfd, QSocketNotifier::Read);
	connect(snr, SIGNAL(activated(int)), this, SLOT(slotReceived(int)));
	
	wMain->updateConnectionStatus();
}

void PClient::slotReceived(int n)
{	
	if (server_handle() <= 0)
	{
		delete snr;
		socket_close(srv.sock);
		connected = false;
		
		wMain->addLog(tr("Connection closed."));
		wMain->updateConnectionStatus();
	}
	
#ifdef PLATFORM_WINDOWS
	// http://doc.trolltech.com/4.3/qsocketnotifier.html#notes-for-windows-users
	snr->setEnabled(true);
#endif
}

void PClient::chatAll(QString text)
{
	if (!connected)
		return;
	
	char msg[1024];
	snprintf(msg, sizeof(msg), "CHAT %d %s", -1, text.toStdString().c_str());
	send_msg(msg);
}

int main(int argc, char **argv)
{
	dbg_print("main", "HoldingNuts pclient (version %d.%d.%d)",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
	
#if not defined(PLATFORM_WINDOWS)
	// ignore broken-pipe signal eventually caused by sockets
	signal(SIGPIPE, SIG_IGN);
#endif
	
	network_init();
	
	PClient app(argc, argv);
	int retval = app.exec();
	
	network_shutdown();
	
	return retval;
}
