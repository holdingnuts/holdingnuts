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


#ifndef _PCLIENT_H
#define _PCLIENT_H

#include <string>
#include <vector>
#include <map>

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QSocketNotifier>
#include <QTimer>
#include <QDir>

#include "Network.h"

#include "Card.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "GameLogic.hpp"
#include "Player.hpp"

#include "WMain.hpp"
#include "WTable.hpp"


typedef struct {
	socktype sock;
	
	char msgbuf[1024];
	int buflen;
	
	int cid;   // our client-id assigned by server
} servercon;

typedef struct {
	char name[255];
} playerinfo;


typedef struct {
	bool sitting;
	bool subscribed;
	table_snapshot snap;
	HoleCards holecards;
	WTable *window;
} tableinfo;

typedef struct {
	bool registered;
	std::map<int,tableinfo> tables;
} gameinfo;



class PClient : public QApplication
{
Q_OBJECT

public:
	PClient(int &argc, char **argv);
	
	bool doConnect(QString strServer, unsigned int port);
	void doClose();
	
	bool isConnected() const { return connected || connecting; };
	void chatAll(QString text);
	
	bool doSetAction(int gid, Player::PlayerAction action, float amount=0.0f);
	
	void doRegister(int gid);
	
	int getTableInfo(int gid, int tid, tableinfo *info);
	int getPlayerInfo(int cid, playerinfo *info);
	int getMyCId();
	
	WMain *wMain;
	
private:
	
	int connectfd;
	QSocketNotifier *snw, *snr;
	
	QTimer *connect_timer;
	bool connected;
	bool connecting;

private slots:
	void slotConnected(int n);
	void slotReceived(int n);
	void slotConnectTimeout();
};

#endif /* _PCLIENT_H */
