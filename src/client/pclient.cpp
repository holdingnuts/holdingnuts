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

#include "Config.h"
#include "Logger.h"
#include "Debug.h"
#include "SysAccess.h"
#include "Tokenizer.hpp"
#include "ConfigParser.hpp"

#include "Protocol.h"
#include "Table.hpp"   // needed for reading snapshots // FIXME: should be all in protocol.h

#include "pclient.hpp"

#include <cstdlib>
#include <cstdio>
#include <string>

#if not defined(PLATFORM_WINDOWS)
# include <signal.h>
#endif

//////////////

ConfigParser config;

typedef std::map<int,playerinfo>	players_type;
typedef std::map<int,gameinfo>		games_type;

servercon		srv;
players_type		players;
games_type		games;

//////////////

int send_msg(const char *msg)
{
	char buf[1024];
	const int len = snprintf(buf, sizeof(buf), "%s\n", msg);
	const int bytes = socket_write(srv.sock, buf, len);
	
	// FIXME: send remaining bytes if not all have been sent
	
	if (len != bytes)
		dbg_print("connectsock", "(%d) warning: not all bytes written (%d != %d)", srv.sock, len, bytes);
	
	return bytes;
}

// server command PSERVER <version> <client-id>
void server_cmd_pserver(Tokenizer& t)
{
	unsigned int version = t.getNextInt();
	srv.cid = t.getNextInt();
		
	dbg_print("server", "Server running version %d.%d.%d. Your client ID is %d",
			  VERSION_GETMAJOR(version), VERSION_GETMINOR(version), VERSION_GETREVISION(version),
							   srv.cid);

	char msg[1024];
	// send user info
	snprintf(msg, sizeof(msg), "INFO name:%s",
			 ((PClient*)qApp)->wMain->getUsername().toStdString().c_str());
		
	send_msg(msg);
}

// server command ERR [<code>] [<text>]
void server_cmd_error(Tokenizer& t)
{
	dbg_print("server", "last cmd error!");

	const int err_code = t.getNextInt();
	const QString qmsg(QString::fromStdString(t.getTillEnd()));

	((PClient*)qApp)->wMain->addServerErrorMessage(err_code, qmsg);
}

// server command MSG <from> <sender-name> <message>
// from := { -1 | <game-id>:<table-id> | <client-id> | <gid>:<tid>:<cid> }   # -1 server
void server_cmd_msg(Tokenizer& t)
{
	const std::string idfrom = t.getNext();

	Tokenizer ft;
	ft.parse(idfrom, ":");
		
	int from = -1;
	int gid = -1;
	int tid = -1;
	int cid = -1;

	if (ft.count() == 2)
	{
		gid = ft.getNextInt();
		tid = ft.getNextInt();
	}
	else if (ft.count() == 3)
	{
		gid = ft.getNextInt();
		tid = ft.getNextInt();
		cid = ft.getNextInt();
	}
	else
		from = ft.getNextInt();

	// playername
	const QString qsfrom(QString::fromStdString(t.getNext()));
	// chatmessage
	QString qchatmsg(QString::fromStdString(t.getTillEnd()));

	// replace all occurrences of "[cid]" in server-msg with player-name
	if (from == -1)
	{
		QRegExp rx("\\[(\\d+)\\]");
			
		while(rx.indexIn(qchatmsg) != -1)
		{
			QString name = "???";
			players_type::iterator it = players.find(rx.cap(1).toInt());

			if(it != players.end())
				name = it->second.name;
				
			qchatmsg.replace(rx, "'" + name + "'");
		}
	}

	if (gid != -1 && tid != -1)
	{
		tableinfo* tinfo = ((PClient*)qApp)->getTableInfo(gid, tid);

		Q_ASSERT_X(tinfo, __func__, "getTableInfo failed");
			
		if (cid == -1) // message from server to table
			tinfo->window->addServerMessage(qchatmsg);
		else // message from user to table
			tinfo->window->addChat(qsfrom, qchatmsg);
	}
	else
	{
		if (from == -1) // message from server to foyer
			((PClient*)qApp)->wMain->addServerMessage(qchatmsg);
		else // message from user to foyer
			((PClient*)qApp)->wMain->addChat(qsfrom, qchatmsg);
	}
} // server_cmd_msg

int server_execute(const char *cmd)
{
#ifdef DEBUG	
//	dbg_print("server_execute", "cmd= %s", cmd);
#endif

	char msg[1024];
	
	Tokenizer t;
	t.parse(cmd);  // parse the command line
	
	if (!t.count())
		return 0;
	
	// get first arg
	std::string command = t.getNext();
	t.popFirst();   // remove the command token
	
	// FIXME: state; check if this is really a pserver
	if (command == "PSERVER")
		server_cmd_pserver(t);
	else if (command == "OK")
	{
		
	}
	else if (command == "ERR")
		server_cmd_error(t);
	else if (command == "MSG")
		server_cmd_msg(t);
	else if (command == "SNAP")		// TODO: snap only debug-code
	{
		std::string from = t.getNext();
		Tokenizer ft;
		ft.parse(from, ":");
		int gid = ft.getNextInt();
		int tid = ft.getNextInt();
		
		snaptype snap = (snaptype)t.getNextInt();
		
		switch ((int)snap)
		{
		case SnapGameState:
			{
				std::string sstate = t.getNext();
				
				if (sstate == "start")
				{
					((PClient*)qApp)->wMain->addServerMessage("Game has been started");
					
					games[gid].registered = true;
					games[gid].tables[tid].sitting = true;
					games[gid].tables[tid].subscribed = true;
					games[gid].tables[tid].window = new WTable(gid, tid);
					
					// show table after some delay (give time to retrieve player-info)
					QTimer::singleShot(2000, games[gid].tables[tid].window, SLOT(slotShow()));
					
					// request the player-list of the game
					char msg[1024];
					snprintf(msg, sizeof(msg), "REQUEST playerlist %d",
						gid);
					send_msg(msg);
				}
				else if (sstate == "end")
					dbg_print("game", "game ended");
			}
			break;
		case SnapTable:
			{
				#ifdef DEBUG
				dbg_print("snap", "%s", cmd);
				#endif
				
				tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(gid, tid);
				
				if (!tinfo)
					return 0;
				
				table_snapshot &table = tinfo->snap;
				HoleCards &holecards = tinfo->holecards;
				
				Tokenizer st;
				
				// state:betting_round
				std::string tmp = t.getNext();
				st.parse(tmp, ":");
				
				table.state = st.getNextInt();
				table.betting_round = st.getNextInt();
				
				// dealer:sb:bb:current
				tmp = t.getNext();
				st.parse(tmp, ":");
				table.s_dealer = st.getNextInt();
				table.s_sb = st.getNextInt();
				table.s_bb = st.getNextInt();
				table.s_cur = st.getNextInt();
				table.s_lastbet = st.getNextInt();
				
				// community-cards
				{
					//table.communitycards = t.getNext().substr(3);
					std::string board = t.getNext().substr(3);
					CommunityCards &cc = table.communitycards;
					
					Tokenizer ct;
					ct.parse(board, ":");
					
					if (ct.count() == 0)
						cc.clear();
					
					if (ct.count() >= 3)
					{
						Card cf1(ct.getNext().c_str());
						Card cf2(ct.getNext().c_str());
						Card cf3(ct.getNext().c_str());
						
						cc.setFlop(cf1, cf2, cf3);
					}
					
					if (ct.count() >= 4)
					{
						Card ct1(ct.getNext().c_str());
						
						cc.setTurn(ct1);
					}
					
					if (ct.count() == 5)
					{
						Card cr1(ct.getNext().c_str());
						
						cc.setRiver(cr1);
					}
				}
				
				// table.seats
				table.my_seat = -1;
				
				const unsigned int seat_max = 10;
				memset(table.seats, 0, seat_max*sizeof(seatinfo));
				
				tmp = t.getNext();
				do {
					//dbg_print("seat", "%s", tmp.c_str());
					
					Tokenizer st;
					st.parse(tmp, ":");
					
					unsigned int seat_no = Tokenizer::string2int(st.getNext().substr(1));
					
					seatinfo si;
					memset(&si, 0, sizeof(si));
					
					si.valid = true;
					si.client_id = st.getNextInt();
					
					if (si.client_id == srv.cid)
						table.my_seat = seat_no;
					
					int pstate = st.getNextInt();
					if (pstate & PlayerInRound)
						si.in_round = true;
					// FIXME: player-sitout
					
					si.stake = st.getNextFloat();
					si.bet = st.getNextFloat();
					si.action = (Player::PlayerAction) st.getNextInt();
					
					std::string shole = st.getNext();
					if (shole.length() == 4)
					{
						Card h1(shole.substr(0, 2).c_str());
						Card h2(shole.substr(2, 2).c_str());
						si.holecards.setCards(h1, h2);
					}
					else
						si.holecards.clear();
					
					if (seat_no < seat_max)
						table.seats[seat_no] = si;
					
					tmp = t.getNext();
				} while (tmp[0] == 's');
				
				
				table.pots.clear();
				
				// pots
				do {
					//dbg_print("pot", "%s", tmp.c_str());
					Tokenizer pt;
					pt.parse(tmp, ":");
					
					pt.getNext();   // pot-no; unused
					float potsize = pt.getNextFloat();
					table.pots.push_back(potsize);
					
					tmp = t.getNext();
				} while (tmp[0] == 'p');
				
				
				table.minimum_bet = Tokenizer::string2float(tmp);
				
				
				if (table.state == Table::Blinds)
				{
					holecards.clear();
				}
				
				if (tinfo->window)
					tinfo->window->updateView();
			}
			break;
		
		case SnapHoleCards:
			{
				tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(gid, tid);
				
				if (!tinfo)
					return 0;
				
				HoleCards &h = tinfo->holecards;
				
				std::string hole = t.getNext();
				
				Tokenizer ct;
				ct.parse(hole, ":");
				
				Card ch1(ct.getNext().c_str());
				Card ch2(ct.getNext().c_str());
				
				h.setCards(ch1, ch2);
				
				if (tinfo->window)
					tinfo->window->updateView();
			}
			break;
		
		}
	}
	else if (command == "PLAYERLIST")
	{
		/* from */ t.getNextInt();
		
		std::string sreq = t.getTillEnd();
		
		snprintf(msg, sizeof(msg), "REQUEST clientinfo %s", sreq.c_str());
		send_msg(msg);
	}
	else if (command == "CLIENTINFO")
	{
		int cid = t.getNextInt();
		
		playerinfo pi;
		memset(&pi, 0, sizeof(pi));
		
		std::string sinfo;
		while (t.getNext(sinfo))
		{
			Tokenizer it;
			it.parse(sinfo, ":");
			
			std::string itype = it.getNext();
			std::string ivalue = it.getNext();
			
			if (itype == "name")
			{
				snprintf(pi.name, sizeof(pi.name), "%s", ivalue.c_str());
			}
		}
		
		//update_player_info(&pi);
		players[cid] = pi;
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
	
//	dbg_print("connectsock", "(%d) DATA len=%d", sock, bytes);

	if (srv.buflen + bytes > (int)sizeof(srv.msgbuf))
	{
		dbg_print("connectsock", "(%d) error: buffer size exceeded", sock);
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

void PClient::doRegister(int gid)
{
	char msg[1024];
	
	if (!connected)
		return;
	
	snprintf(msg, sizeof(msg), "REGISTER %d",
		gid);
		
	send_msg(msg);
}

bool PClient::doSetAction(int gid, Player::PlayerAction action, float amount)
{
	if (!connected)
		return false;
	
	const char *saction = "";
	switch ((int) action)
	{
	case Player::Fold:
		saction = "fold";
		break;
	case Player::Call:
		saction = "call";
		break;
	case Player::Raise:
		saction = "raise";
		break;
	case Player::Allin:
		saction = "allin";
		break;
	case Player::Show:
		saction = "show";
		break;
	case Player::Muck:
		saction = "muck";
		break;
	case Player::ResetAction:
		saction = "reset";
		break;
	}
	
	char msg[1024];
	snprintf(msg, sizeof(msg), "ACTION %d %s %0.2f",
		gid, saction, amount);
	
	send_msg(msg);
	
	return true;
}

void PClient::slotConnectTimeout()
{
	wMain->addLog(tr("Connection timeout."));
	
	delete snw;
	delete connect_timer;      // FIXME: can't delete on event handler
	
	connected = false;
	connecting = false;
	
	wMain->updateConnectionStatus();
}

void PClient::slotConnected(int n)
{
	snw->setEnabled(false);
	
	wMain->addLog(tr("Connected."));
	
	delete snw;   // FIXME: can't delete on event handler
	delete connect_timer;
	
	memset(&srv, 0, sizeof(srv));
	srv.sock = connectfd;
	
	connected = true;
	connecting = false;
	
	snr = new QSocketNotifier(connectfd, QSocketNotifier::Read);
	connect(snr, SIGNAL(activated(int)), this, SLOT(slotReceived(int)));
	
	wMain->updateConnectionStatus();
	
	
	// send protocol introduction
	char msg[1024];
	snprintf(msg, sizeof(msg), "PCLIENT %d %s",
		VERSION_CREATE(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION),
		config.get("uuid").c_str());
	
	send_msg(msg);
}

void PClient::slotReceived(int n)
{
#ifdef PLATFORM_WINDOWS
	// http://doc.trolltech.com/4.4/qsocketnotifier.html#notes-for-windows-users
	snr->setEnabled(false);
#endif
	
	const int status = server_handle();
	
	if (status <= 0)
	{
		wMain->addLog(tr("Connection closed."));
		
		delete snr;    // FIXME: can't delete on event handler
		socket_close(srv.sock);
		connected = false;
		
		wMain->updateConnectionStatus();
	}
	
#ifdef PLATFORM_WINDOWS
	// http://doc.trolltech.com/4.4/qsocketnotifier.html#notes-for-windows-users
	snr->setEnabled(true);
#endif
}

void PClient::chatAll(const QString& text)
{
	// foyer chat
	if (!connected)
		return;
	
	char msg[1024];
	snprintf(msg, sizeof(msg), "CHAT %d %s", -1, text.simplified().toStdString().c_str());
	send_msg(msg);
}

void PClient::chat(const QString& text, int gid, int tid)
{
	if (!connected)
		return;

	char msg[1024];
	snprintf(msg, sizeof(msg), "CHAT %d:%d %s", gid, tid, text.simplified().toStdString().c_str());
	send_msg(msg);
}

tableinfo* PClient::getTableInfo(int gid, int tid)
{
	if (games.find(gid) != games.end() && games[gid].tables.find(tid) != games[gid].tables.end())
		return &(games[gid].tables[tid]);
	else
		return 0;
}

playerinfo* PClient::getPlayerInfo(int cid)
{
	if (players.find(cid) != players.end())
		return &(players[cid]);
	else
		return 0;
}

int PClient::getMyCId()
{
	return srv.cid;
}

PClient::PClient(int &argc, char **argv) : QApplication(argc, argv)
{
	connected = false;
	connecting = false;
	
	// change into data-dir
	QDir::setCurrent("data");
	
	QString locale;
	if (config.get("locale").length())
		locale = QString::fromStdString(config.get("locale"));
	else
		locale = QLocale::system().name().left(2);
	
	QTranslator myappTranslator;
	myappTranslator.load("i18n/hn_" + locale);
	dbg_print("main", "Using locale: %s", locale.toStdString().c_str());
	installTranslator(&myappTranslator);
	
	wMain = new WMain();
	wMain->updateConnectionStatus();
	wMain->show();

#ifdef DEBUG	
	// parse commandline arguments in debugmode
	if (argc == 2)
	{
		if (strcmp(argv[1], "--dbg_register") == 0)
		{
			doConnect("localhost", DEFAULT_SERVER_PORT);
			// TODO: wait for n seconds and then try to register a game
			doRegister(0);
		}
	}
#endif
}

bool config_load()
{
	// include config defaults
	#include "client_variables.hpp"
	
	// create config-dir if it doesn't yet exist
	sys_mkdir(sys_config_path());
	
	char cfgfile[1024];
	snprintf(cfgfile, sizeof(cfgfile), "%s/client.cfg", sys_config_path());
	
	if (config.load(cfgfile))
		dbg_print("config", "Loaded configuration from %s", cfgfile);
	else
	{
		if (config.save(cfgfile))
			dbg_print("config", "Saved initial configuration to %s", cfgfile);
	}
	
	return true;
}

int main(int argc, char **argv)
{
	dbg_setlog(stderr, 0);
	
	dbg_print("main", "HoldingNuts pclient (version %d.%d.%d; Qt version %s)",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION,
		qVersion());
	
#if not defined(PLATFORM_WINDOWS)
	// ignore broken-pipe signal eventually caused by sockets
	signal(SIGPIPE, SIG_IGN);
#endif
	
	// load config
	config_load();
	
	// start logging
	filetype *fplog = NULL;
	if (config.getBool("log"))
	{
		char logfile[1024];
		snprintf(logfile, sizeof(logfile), "%s/client.log", sys_config_path());
		fplog = file_open(logfile, mode_write);
		
		dbg_setlog(stderr, fplog);
	}
	
	network_init();
	
	PClient app(argc, argv);
	int retval = app.exec();
	
	network_shutdown();
	
	// close log-file
	if (fplog)
		file_close(fplog);
	
	return retval;
}
