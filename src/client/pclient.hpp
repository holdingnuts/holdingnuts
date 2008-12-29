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

#include <QApplication>
#include <QSocketNotifier>
#include <QTimer>

#include "WMain.hpp"

class PClient : public QApplication
{
Q_OBJECT

public:
	PClient(int &argc, char **argv);
	
	bool doConnect(QString strServer, unsigned int port);
	void doClose();
	
	bool isConnected() const { return connected || connecting; };
	void chatAll(QString text);
	
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
