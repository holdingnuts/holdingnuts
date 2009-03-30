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
#include "ConfigParser.hpp"

#include "Table.hpp"   // needed for reading snapshots // FIXME: should be all in protocol.h

#include "pclient.hpp"

#include <cstdlib>
#include <cstdio>
#include <string>

#include <QUuid>
#include <QMessageBox>
#include <QDateTime>

#ifndef NOAUDIO
# include "Audio.h"
#endif
#include "data.h"

//////////////

ConfigParser config;

static servercon		srv;
static players_type		players;
static games_type		games;
static gamelist_type		gamelist;

//////////////

// server command PSERVER <version> <client-id>
void PClient::serverCmdPserver(Tokenizer &t)
{
	const unsigned int version = t.getNextInt();
	srv.cid = t.getNextInt();
	
	const uint time_remote = t.getNextInt();
	srv.time_remote_delta = time_remote - QDateTime::currentDateTime().toTime_t();
	
	srv.introduced = true;
		
	log_msg("server", "Server running version %d.%d.%d. Your client ID is %d",
			  VERSION_GETMAJOR(version), VERSION_GETMINOR(version), VERSION_GETREVISION(version),
							   srv.cid);
	
	
	// there is a newer version available
	if (version > VERSION)
	{
		const QString sversion =
			tr("There is a newer version of HoldingNuts available (at least %1.%2.%3)")
				.arg(VERSION_GETMAJOR(version))
				.arg(VERSION_GETMINOR(version))
				.arg(VERSION_GETREVISION(version));
		wMain->addServerMessage(sversion);
	}
	
	
	// send user info
	char msg[1024];
	snprintf(msg, sizeof(msg), "INFO \"name:%s\" \"location:%s\"",
		  config.get("player_name").c_str(),
		  config.get("info_location").c_str());
		
	netSendMsg(msg);
	
	// request initial game-list and start update-timer
	requestGamelist();
}

// server command ERR [<code>] [<text>]
void PClient::serverCmdErr(Tokenizer &t)
{
	const int err_code = t.getNextInt();
	const std::string smsg = t.getTillEnd();
	
	log_msg("cmd", "error executing command (id=%d code=%d): %s",
		srv.last_msgid, err_code, smsg.c_str());
	
	wMain->addServerErrorMessage(err_code, QString::fromStdString(smsg));
	
	// check for version mismatch
	if (err_code == ErrWrongVersion)
		QMessageBox::critical(wMain, "Error",
			tr("The version of this client isn't compatible anymore "
				"with the server. Please download a recent version."));
}

// server command MSG <from> <sender-name> <message>
// from := { -1 | <game-id>:<table-id> | <client-id> | <gid>:<tid>:<cid> }   # -1 server
void PClient::serverCmdMsg(Tokenizer &t)
{
	const std::string idfrom = t.getNext();

	Tokenizer ft(":");
	ft.parse(idfrom);
		
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
			
			qchatmsg.replace(rx, name);
		}
	}

	if (gid != -1 && tid != -1)
	{
		tableinfo* tinfo = getTableInfo(gid, tid);
		
		// silently drop message if there is no table-info
		if (!tinfo)
			return;
		
		if (cid == -1) // message from server to table
			tinfo->window->addServerMessage(qchatmsg);
		else // message from user to table
			tinfo->window->addChat(qsfrom, qchatmsg);
	}
	else
	{
		if (from == -1) // message from server to foyer
			wMain->addServerMessage(qchatmsg);
		else // message from user to foyer
			wMain->addChat(qsfrom, qchatmsg);
	}
}

void PClient::serverCmdSnap(Tokenizer &t)
{
	std::string from = t.getNext();
	Tokenizer ft(":");
	ft.parse(from);
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
				wMain->addServerMessage(
					QString(tr("Game (%1) has been started.").arg(gid)));
				
				addTable(gid, tid);
			}
			else if (sstate == "end")
			{
				wMain->addServerMessage(
					QString(tr("Game (%1) has been ended.").arg(gid)));
				
				// FIXME: remove game
			}
		}
		break;
	case SnapTable:
		{
			tableinfo *tinfo = getTableInfo(gid, tid);
			
			if (!tinfo)
			{
				return;
			}
			
			table_snapshot &table = tinfo->snap;
			HoleCards &holecards = tinfo->holecards;
			
			Tokenizer st(":");
			
			// state:betting_round
			std::string tmp = t.getNext();
			st.parse(tmp);
			
			table.state = st.getNextInt();
			table.betting_round = st.getNextInt();
			
			// dealer:sb:bb:current
			tmp = t.getNext();
			st.parse(tmp);
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
				
				Tokenizer ct(":");
				ct.parse(board);
				
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
				//log_msg("seat", "%s", tmp.c_str());
				
				Tokenizer st(":");
				st.parse(tmp);
				
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
				if (pstate & PlayerSitout)
					si.sitout = true;
				
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
				//log_msg("pot", "%s", tmp.c_str());
				Tokenizer pt(":");
				pt.parse(tmp);
				
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
			bool bUpdateView = true;
			tableinfo *tinfo = getTableInfo(gid, tid);
			
			if (!tinfo)
			{
				addTable(gid, tid);
				tinfo = getTableInfo(gid, tid);
				
				bUpdateView = false;
			}
			
			HoleCards &h = tinfo->holecards;
			
			std::string hole = t.getNext();
			
			Tokenizer ct(":");
			ct.parse(hole);
			
			Card ch1(ct.getNext().c_str());
			Card ch2(ct.getNext().c_str());
			
			h.setCards(ch1, ch2);
			
			if (bUpdateView && tinfo->window)
				tinfo->window->updateView();
		}
		break;
	}
}

// server command PLAYERLIST <gid> <client-id> [...]
void PClient::serverCmdPlayerlist(Tokenizer &t)
{
	char msg[1024];
	
	const int gid = t.getNextInt();

	// find game
	games_type::iterator git = games.find(gid);
	
	Q_ASSERT_X(git != games.end(), Q_FUNC_INFO, "game not found");
	
	// clear current player list
	git->second.players.clear();
	
	// client-list
	std::string sreq_orig = t.getTillEnd();
	std::string sreq_clean;
	
	Tokenizer tcid(" ");
	tcid.parse(sreq_orig);
	
	std::string token;
	while (tcid.getNext(token))
	{
		unsigned int cid = Tokenizer::string2int(token);
		
		// append cid to player list
		git->second.players.push_back(cid);
		
		// skip this client for the request... if we already know him
		if (getPlayerInfo(cid))
				continue;
		
		sreq_clean += token + " ";
	}
	
	// only request client-info if there are unknown clients left
	if (sreq_clean.length())
	{
		snprintf(msg, sizeof(msg), "REQUEST clientinfo %s", sreq_clean.c_str());
		netSendMsg(msg);
	}
	
	wMain->notifyPlayerlist(gid);
}

// server command CLIENTINFO <client-id> <type>:<value> [...]
void PClient::serverCmdClientinfo(Tokenizer &t)
{
	int cid = t.getNextInt();
	
	playerinfo pi;
	
	std::string sinfo;
	while (t.getNext(sinfo))
	{
		Tokenizer it(":");
		it.parse(sinfo);
		
		std::string itype = it.getNext();
		std::string ivalue = it.getNext();
		
		if (itype == "name")
			pi.name = QString::fromStdString(ivalue);
		else if (itype == "location")
			pi.location = QString::fromStdString(ivalue);
	}
	
	//update_player_info(&pi);
	players[cid] = pi;
	
	Q_ASSERT_X(wMain, Q_FUNC_INFO, "invalid mainwindow pointer");
	
	wMain->notifyPlayerinfo(cid);
}

// server command GAMEINFO <gid> <type>:<value> [...]
void PClient::serverCmdGameinfo(Tokenizer &t)
{
	int gid = t.getNextInt();
	
	games_type::iterator git = games.find(gid);
	
	if (git == games.end())
	{
		// if no game found add a new one to the list
		git = games.insert(
			git, 
			games_type::value_type(gid, games_type::mapped_type()));
	}
	
	gameinfo *gi = &(git->second);
	
	
	// unpack info
	const std::string sinfo = t.getNext();
	
	Tokenizer it(":");
	it.parse(sinfo);
	
	gi->type = (gametype) it.getNextInt();
	gi->mode = (gamemode) it.getNextInt();
	gi->state = (gamestate) it.getNextInt();
	gi->players_max = it.getNextInt();
	gi->players_count = it.getNextInt();
	gi->player_timeout = it.getNextInt();
	gi->initial_stakes = it.getNextFloat();
	
	
	// unpack blinds-rule
	const std::string sblinds = t.getNext();
	it.parse(sblinds);
	
	gi->blinds_start = it.getNextFloat();
	gi->blinds_factor = it.getNextFloat();
	gi->blinds_time = it.getNextInt();
	
	
	// game name
	gi->name = QString::fromStdString(t.getNext());
	
	
	// notify WMain there's an updated gameinfo available
	Q_ASSERT_X(wMain, Q_FUNC_INFO, "invalid mainwindow pointer");

	wMain->notifyGameinfo(gid);
}

// server command GAMELIST <gid> [...]
void PClient::serverCmdGamelist(Tokenizer &t)
{
	// game-list
	std::string sreq;
	
	gamelist.clear();
	
	std::string sgid;
	while (t.getNext(sgid))
	{
		gamelist.push_back(Tokenizer::string2int(sgid));
		sreq += sgid + " ";
	}
	
	// get game info
	requestGameinfo(sreq.c_str());
	
	wMain->notifyGamelist();
}

int PClient::serverExecute(const char *cmd)
{
	Tokenizer t(" ");
	t.parse(cmd);  // parse the command line
	
	if (!t.count())
		return 0;
	
#ifdef DEBUG
	if (config.getBool("dbg_srv_cmd"))
		dbg_msg("server_execute", "cmd= %s", cmd);
#endif
	
	// extract message-id if present
	const char firstchar = t[0][0];
	if (firstchar >= '0' && firstchar <= '9')
		srv.last_msgid = t.getNextInt();
	else
		srv.last_msgid = -1;
	
	
	// get command argument
	const std::string command = t.getNext();
	
	
	if (!srv.introduced)   // state: not introduced
	{
		if (command == "PSERVER")
			serverCmdPserver(t);
		else if (command == "ERR")
		{
			serverCmdErr(t);
			
			doClose();
		}
		else if (command == "OK")
		{
			// do nothing
		}
		else
		{
			// protocol error: maybe this isn't a pserver
			log_msg("introduce", "protocol error");
			wMain->addServerErrorMessage(
				ErrProtocol, tr("Protocol error. The remote host does not seem to be a HoldingNuts server."));
			
			// FIXME: don't do a regular/clean close (avoid the QUIT sequence)
			doClose();
		}
	}
	else if (command == "OK")
	{
		
	}
	else if (command == "ERR")
		serverCmdErr(t);
	else if (command == "MSG")
		serverCmdMsg(t);
	else if (command == "SNAP")
		serverCmdSnap(t);
	else if (command == "PLAYERLIST")
		serverCmdPlayerlist(t);
	else if (command == "CLIENTINFO")
		serverCmdClientinfo(t);
	else if (command == "GAMEINFO")
		serverCmdGameinfo(t);
	else if (command == "GAMELIST")
		serverCmdGamelist(t);

	return 0;
}

// returns zero if no cmd was found or no bytes remaining after exec
int PClient::serverParsebuffer()
{
	//log_msg("clientsock", "(%d) parse (bufferlen=%d)", srv.sock, srv.buflen);
	
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
		
		//log_msg("clientsock", "(%d) command: '%s' (len=%d)", srv.sock, cmd, found_nl);
		if (serverExecute(cmd) != -1)
		{
			// move the rest to front
			memmove(srv.msgbuf, srv.msgbuf + found_nl + 1, srv.buflen - (found_nl + 1));
			srv.buflen -= found_nl + 1;
			//log_msg("clientsock", "(%d) new buffer after cmd (bufferlen=%d)", srv.sock, srv.buflen);
			
			retval = srv.buflen;
		}
		else
			retval = 0;
	}
	else
		retval = 0;
	
	return retval;
}

bool PClient::addTable(int gid, int tid)
{
	gameinfo *game = getGameInfo(gid);
	
	if (!game)
	{
		games[gid];  // FIXME: better way of adding item
		game = getGameInfo(gid);
		
		game->registered = true;
	}
	
	tableinfo *table = getTableInfo(gid, tid);
	
	if (!table)
	{
		game->tables[tid];  // FIXME: better way of adding item
		table = getTableInfo(gid, tid);
		
		table->sitting = true;
		table->subscribed = true;
		table->window = new WTable(gid, tid);
		
		// show table after some delay (give time to retrieve player-info)
		QTimer::singleShot(2000, table->window, SLOT(slotShow()));
	}
	
	
	// request the gameinfo
	requestGameinfo(gid);
	
	// request the player-list of the game
	requestPlayerlist(gid);
	
	return true;
}

bool PClient::doConnect(QString strServer, unsigned int port)
{
	log_msg("net", "Connecting to %s:%d...", strServer.toStdString().c_str(), port);
	wMain->addLog(tr("Connecting..."));
	
	tcpSocket->connectToHost(strServer, port);
	
	connecting = true;
	
	wMain->updateConnectionStatus();
	
	return true;
}

void PClient::doClose()
{
	if (!connecting)
		netSendMsg("QUIT");
	
	tcpSocket->abort();
	tcpSocket->close();
}

void PClient::doRegister(int gid, bool bRegister)
{
	char msg[1024];
	
	if (!connected)
		return;
	
	if (bRegister)
		snprintf(msg, sizeof(msg), "REGISTER %d", gid);
	else
		snprintf(msg, sizeof(msg), "UNREGISTER %d", gid);
		
	netSendMsg(msg);
}

bool PClient::doSetAction(int gid, Player::PlayerAction action, float amount)
{
	if (!connected)
		return false;
	
	const char *saction = "";
	bool bAmount = false;
	
	switch ((int) action)
	{
	case Player::Fold:
		saction = "fold";
		break;
	case Player::Call:
		saction = "call";
		bAmount = true;
		break;
	case Player::Raise:
		saction = "raise";
		bAmount = true;
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
	case Player::Back:
		saction = "back";
		break;
	case Player::Sitout:
		saction = "sitout";
		break;
	}
	
	char msg[1024];
	if (bAmount)
		snprintf(msg, sizeof(msg), "ACTION %d %s %0.2f",
			gid, saction, amount);
	else
		snprintf(msg, sizeof(msg), "ACTION %d %s",
			gid, saction);
	
	netSendMsg(msg);
	
	return true;
}

void PClient::chatAll(const QString& text)
{
	// foyer chat
	if (!connected)
		return;
	
	char msg[1024];
	snprintf(msg, sizeof(msg), "CHAT %d %s", -1, text.simplified().toStdString().c_str());
	netSendMsg(msg);
}

void PClient::chat(const QString& text, int gid, int tid)
{
	if (!connected)
		return;

	char msg[1024];
	snprintf(msg, sizeof(msg), "CHAT %d:%d %s", gid, tid, text.simplified().toStdString().c_str());
	netSendMsg(msg);
}

bool PClient::createGame(gamecreate *createinfo)
{
	char msg[1024];
	snprintf(msg, sizeof(msg), "CREATE players:%d stake:%.2f timeout:%d "
		"blinds_start:%.2f blinds_factor:%.2f blinds_time:%d "
		"\"name:%s\"",
		createinfo->max_players,
		createinfo->stake,
		createinfo->timeout,
		createinfo->blinds_start,
		createinfo->blinds_factor,
		createinfo->blinds_time,
		createinfo->name.simplified().toStdString().c_str());
	netSendMsg(msg);
	
	return true;
}

#if 0
const gamelist_type& PClient::getGameList()
{
	return gamelist;
}
#endif

gameinfo* PClient::getGameInfo(int gid)
{
	if (games.find(gid) != games.end())
		return &(games[gid]);
	else
		return 0;
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

#ifdef DEBUG
void PClient::slotDbgRegister()
{
	const int gid = config.getInt("dbg_register");
	doRegister(gid);
}
#endif

void PClient::sendDebugMsg(const QString& msg)
{
	netSendMsg(msg.toStdString().c_str());
}

QDateTime PClient::getServerTime()
{
	if (!connected)
		return QDateTime();
	
	QDateTime timeServer;
	timeServer.setTime_t(QDateTime::currentDateTime().toTime_t() + srv.time_remote_delta);
	return timeServer;
}

int PClient::netSendMsg(const char *msg)
{
	char buf[1024];
	const int len = snprintf(buf, sizeof(buf), "%s\n", msg);
	const int bytes = tcpSocket->write(buf, len);
	
	// FIXME: send remaining bytes if not all have been sent
	
	if (len != bytes)
		log_msg("connectsock", "warning: not all bytes written (%d != %d)", len, bytes);
	
	return bytes;
}

void PClient::netRead()
{
	char buf[1024];
	int bytes;
	
	do
	{
		// return early if there's nothing to read
		if ((bytes = tcpSocket->read(buf, sizeof(buf))) <= 0)
			return;
		
		//log_msg("connectsock", "(%d) DATA len=%d", sock, bytes);
		
		if (srv.buflen + bytes > (int)sizeof(srv.msgbuf))
		{
			log_msg("connectsock", "error: buffer size exceeded");
			srv.buflen = 0;
		}
		else
		{
			memcpy(srv.msgbuf + srv.buflen, buf, bytes);
			srv.buflen += bytes;
			
			// parse and execute all commands in queue
			while (serverParsebuffer());
		}
	} while (sizeof(buf) == bytes);
	
	return;
}

void PClient::netError(QAbstractSocket::SocketError socketError)
{
	log_msg("net", "Connection error: %s", tcpSocket->errorString().toStdString().c_str());
	wMain->addLog(tr("Connection error: %1.").arg(tcpSocket->errorString()));
	
	connected = false;
	connecting = false;
	
	wMain->updateConnectionStatus();
}

void PClient::netConnected()
{
	log_msg("net", "Connection established");
	wMain->addLog(tr("Connected."));
	
	memset(&srv, 0, sizeof(srv));
	
	connected = true;
	connecting = false;
	
	wMain->updateConnectionStatus();
	
	// send protocol introduction
	char msg[1024];
	snprintf(msg, sizeof(msg), "PCLIENT %d %s",
		VERSION,
		config.get("uuid").c_str());
	
	netSendMsg(msg);
}

void PClient::netDisconnected()
{
	log_msg("net", "Connection closed");
	wMain->addLog(tr("Connection closed."));
	
	connected = false;
	connecting = false;
	
	wMain->updateConnectionStatus();
	
	
	// reset all game related data and close table-windows
	for (games_type::iterator e = games.begin(); e != games.end(); e++)
	{
		tables_type &tables = e->second.tables;
		for (tables_type::iterator t = tables.begin(); t != tables.end(); t++)
		{
			tableinfo *table = &(t->second);
			
			if (table->window)
			{
				table->window->close();
				delete table->window;
			}
		}
	}
	
	players.clear();
	games.clear();
	gamelist.clear();
}

void PClient::requestPlayerlist(int gid)
{
	char msg[256];
		
	// request the player-list of the game
	snprintf(msg, sizeof(msg), "REQUEST playerlist %d", gid);
	netSendMsg(msg);
}

void PClient::requestGamelist()
{
	// query gamelist
	netSendMsg("REQUEST gamelist");
}

void PClient::requestGameinfo(const char *glist)
{
	char msg[1024];
	
	// get game infos
	snprintf(msg, sizeof(msg), "REQUEST gameinfo %s", glist);
	netSendMsg(msg);
}

void PClient::requestGameinfo(int gid)
{
	char msg[1024];
	
	// get game info
	snprintf(msg, sizeof(msg), "REQUEST gameinfo %ds", gid);
	netSendMsg(msg);
}

bool PClient::isTableWindowRemaining()
{
	// FIXME: make use of QApplication::lastWindowClosed(), but Qt::WA_QuitOnClose flag needed for table-windows
	
	// check if there are open windows
	for (games_type::iterator e = games.begin(); e != games.end(); e++)
	{
		tables_type &tables = e->second.tables;
		for (tables_type::iterator t = tables.begin(); t != tables.end(); t++)
		{
			tableinfo *table = &(t->second);
			
			if (table->window)
			{
				if (table->window->isVisible())
					return true;
			}
		}
	}
	
	return false;
}

PClient::PClient(int &argc, char **argv) : QApplication(argc, argv)
{
	connected = false;
	connecting = false;
	
	tcpSocket = new QTcpSocket(this);
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(netRead()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
		this, SLOT(netError(QAbstractSocket::SocketError)));
	connect(tcpSocket, SIGNAL(connected()), this, SLOT(netConnected()));
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(netDisconnected()));
	
	// app icon
	Q_INIT_RESOURCE(pclient);
}

PClient::~PClient()
{
	Q_CLEANUP_RESOURCE(pclient);
}

int PClient::init()
{
	// change into data-dir
	const char *datadir = sys_data_path();
	if (datadir)
	{
		log_msg("main", "Using data-directory: %s", datadir);
		sys_chdir(datadir);
	}
	else
	{
		log_msg("main", "Error: data-directory was not found");
		QMessageBox::critical(NULL, "Error", "The data directory was not found.");
		
		return 1;
	}
	
	
	// load locale
	QString locale;
	if (config.get("locale").length())
		locale = QString::fromStdString(config.get("locale"));
	else
	{
		locale = QLocale::system().name().left(2);
		log_msg("main", "Auto-detected locale: %s", locale.toStdString().c_str());
	}
	
	
	if (locale != "en")  // no locale
	{
		QTranslator *translator = new QTranslator();
		if (translator->load("i18n/hn_" + locale))
		{
			installTranslator(translator);
			log_msg("main", "Using locale: %s", locale.toStdString().c_str());
		}
		else
			log_msg("main", "Error: Cannot load locale: %s", locale.toStdString().c_str());
		
		// load qt localization; first try system version, then our own copy
		QTranslator *qt_translator = new QTranslator();
		if (qt_translator->load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath)) ||
			qt_translator->load("i18n/qt_" + locale))
		{
			installTranslator(qt_translator);
		}
	}
	
	
#ifndef NOAUDIO
	// load sounds
	struct sound {
		unsigned int id;
		const char *file;
	} sounds[] = {
		{ SOUND_TEST_1,		"audio/test.wav" },
		{ SOUND_DEAL_1,		"audio/deal.wav" }
	};
	
	const unsigned int sounds_count = sizeof(sounds) / sizeof(sounds[0]);
	
	for (unsigned int i=0; i < sounds_count; i++)
	{
		log_msg("audio", "Loading sound '%s' (%d)", sounds[i].file, sounds[i].id);
		audio_load(sounds[i].id, sounds[i].file);
	}
#endif /* NOAUDIO */
	
	
	// main window
	wMain = new WMain();
	wMain->updateConnectionStatus();
	wMain->show();

#ifdef DEBUG
	if (config.getInt("dbg_register") != -1)
	{
		doConnect(config.get("default_host").c_str(),
			config.getInt("default_port"));
		QTimer::singleShot(1000, this, SLOT(slotDbgRegister()));
	}
#endif
	
	
#if 1
	// FIXME: this is a temporary fix for the C-locale (sprintf-float/strtod) issue
	
	// set libc (and libstdc++) locale to "C"
	std::setlocale(LC_ALL, "C");
	
	//std::locale::global(std::locale("C"));
	//std::cout.imbue(std::locale());
#endif
	
	return 0;
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
		log_msg("config", "Loaded configuration from %s", cfgfile);
	else
	{
		// override defaults
		
		// determine system username and use it as default playername
		const char *name = sys_username();
		if (name)
			config.set("player_name", std::string(name));
		
		// generate an UUID
		QString suuid = QUuid::createUuid().toString();
		suuid = suuid.mid(1, suuid.length() - 2);
		config.set("uuid", suuid.toStdString());
		
		if (config.save(cfgfile))
			log_msg("config", "Saved initial configuration to %s", cfgfile);
	}
	
	return true;
}

int main(int argc, char **argv)
{
	log_set(stdout, 0);
	
	log_msg("main", "HoldingNuts pclient (version %d.%d.%d; svn %s; Qt version %s)",
		VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION,
		VERSIONSTR_SVN,
		qVersion());
	
	
	// load config
	config_load();
	config.print();
	
	// start logging
	filetype *fplog = NULL;
	if (config.getBool("log"))
	{
		char logfile[1024];
		snprintf(logfile, sizeof(logfile), "%s/client.log", sys_config_path());
		fplog = file_open(logfile, mode_write);
		
		log_set(stdout, fplog);
	}
	
#if defined(DEBUG) && defined(PLATFORM_WINDOWS)
	char dbgfile[1024];
	snprintf(dbgfile, sizeof(dbgfile), "%s/client.debug", sys_config_path());
	/*filetype *dbglog = */ file_reopen(dbgfile, mode_write, stderr);  // omit closing
#endif
	
	// initialize SDL for audio
#ifndef NOAUDIO
	audio_init();
#endif
	
	PClient app(argc, argv);
	if (app.init())
		return 1;
	
	int retval = app.exec();
	
	
	// close log-file
	if (fplog)
		file_close(fplog);
	
#ifndef NOAUDIO
	// cleanup SDL
	audio_deinit();
#endif
	
	return retval;
}
