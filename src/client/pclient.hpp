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


#ifndef _PCLIENT_H
#define _PCLIENT_H

#include <string>
#include <vector>
#include <map>

#include <QApplication>
#include <QtNetwork>
#include <QTranslator>
#include <QLocale>
#include <QTimer>
#include <QDir>
#include <QRegExp>

#include "Tokenizer.hpp"

#include "Card.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "GameLogic.hpp"
#include "Player.hpp"

#include "Protocol.h"

#include "WMain.hpp"
#include "WTable.hpp"

typedef struct {
	char msgbuf[1024*256];
	int buflen;
	int last_msgid;
	
	int cid;   // our client-id assigned by server
	
	bool introduced;   // PCLIENT->PSERVER sequence success
	
	uint time_remote_delta;
} servercon;

typedef struct {
	QString name;
	QString location;
} playerinfo;


typedef struct {
	bool sitting;
	bool subscribed;
	table_snapshot snap;
	HoleCards holecards;
	WTable *window;
} tableinfo;

typedef std::map<int,tableinfo>		tables_type;

typedef struct {
	//! \brief local player registered state
 	bool			registered;
	//! \brief name of the game
	QString			name;
	//! \brief gametype
	gametype		type;
	//! \brief gamemode
	gamemode		mode;
	//! \brief gamestate
	gamestate		state;
	//! \brief wherther the game is password protected or not
	bool			password;
	//! \brief wherther the player is owner of this game
	bool			owner;
	//! \brief timeout in seconds if no answer from player
	unsigned int	player_timeout;
	//! \brief current playerscount registered in game
	unsigned int	players_count;
	//! \brief maximum players allowed
	unsigned int	players_max;
	//! \brief registered players
	std::vector<int>	players;
	//! \brief initial player stakes
	chips_type	initial_stakes;
	//! \brief starting blinds
	chips_type	blinds_start;
	//! \brief blinds raise factor
	float		blinds_factor;
	//! \brief blinds raise time
	unsigned int	blinds_time;
	//! \brief known tables
	tables_type		tables;
} gameinfo;

// vector<gid>
typedef std::vector<int>		gamelist_type;
// map<gid, gameinfo>
typedef std::map<int,gameinfo>		games_type;
// map<cid, playerinfo>
typedef std::map<int,playerinfo>	players_type;


typedef struct {
	QString name;
	unsigned int max_players;
	chips_type stake;
	unsigned int timeout;
	unsigned int blinds_time;
	chips_type blinds_start;
	double blinds_factor;
	QString password;
} gamecreate;


class QDateTime;

class PClient : public QApplication
{
Q_OBJECT

public:
	PClient(int &argc, char **argv);
	~PClient();
	
	int init();
	
	bool doConnect(QString strServer, unsigned int port);
	void doClose();
	
	bool isConnected() const { return connected; };
	bool isConnecting() const { return connecting; };
	
	void chatAll(const QString& text);
	void chat(const QString& text, int gid, int tid);
	
	bool createGame(gamecreate *createinfo);
	
	bool doSetAction(int gid, Player::PlayerAction action, chips_type amount=0);
	
	void doRegister(int gid, bool bRegister=true, const QString& password="");
	void doStartGame(int gid);
	
#if 0
	const gamelist_type& getGameList();
#endif
	bool isGameInList(int gid);
	gameinfo* getGameInfo(int gid);
	tableinfo* getTableInfo(int gid, int tid);
	playerinfo* getPlayerInfo(int cid);
	QString getPlayerName(int cid);
	int getMyCId();
	
	WMain* getMainWnd() const { return wMain; };
	bool isTableWindowRemaining();
	
	//! \brief query playerlist from Server
	void requestPlayerlist(int gid);
	
	void requestGameinfo(const char *glist);
	void requestGameinfo(int gid);
	
	void sendDebugMsg(const QString& msg);
	
	QDateTime getServerTime();
	
private:
	WMain *wMain;
		
	bool connected;
	bool connecting;
	
	QTcpSocket	*tcpSocket;
	
private:
	servercon		srv;
	players_type		players;
	games_type		games;
	gamelist_type		gamelist;
private:	
	int netSendMsg(const char *msg);
	
	int serverExecute(const char *cmd);
	int serverParsebuffer();
	
	void serverCmdPserver(Tokenizer &t);
	void serverCmdErr(Tokenizer &t);
	void serverCmdMsg(Tokenizer &t);
	void serverCmdSnap(Tokenizer &t);
	void serverCmdSnapGamestate(Tokenizer &t, int gid, int tid, tableinfo* tinfo);
	void serverCmdSnapTable(Tokenizer &t, int gid, int tid, tableinfo* tinfo);
	void serverCmdSnapCards(Tokenizer &t, int gid, int tid, tableinfo* tinfo);
	void serverCmdSnapPlayerAction(Tokenizer &t, int gid, int tid, tableinfo* tinfo);
	void serverCmdSnapPlayerShow(Tokenizer &t, int gid, int tid, tableinfo* tinfo);
	void serverCmdSnapFoyer(Tokenizer &t);
	void serverCmdPlayerlist(Tokenizer &t);
	void serverCmdClientinfo(Tokenizer &t);
	void serverCmdGameinfo(Tokenizer &t);
	void serverCmdGamelist(Tokenizer &t);
	
	bool addTable(int gid, int tid);

public slots:
	//! \brief query gamelist from Server
	void requestGamelist();
	
private slots:
	void netRead();
	void netError(QAbstractSocket::SocketError socketError);
	void netConnected();
	void netDisconnected();
	
#ifdef DEBUG
	void slotDbgRegister();
#endif
};

#endif /* _PCLIENT_H */
