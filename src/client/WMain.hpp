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


#ifndef _WMAIN_H
#define _WMAIN_H

#include <QMainWindow>

class ChatBox;
class QLineEdit;
class QPushButton;
class QTextEdit;

class WMain : public QMainWindow
{
Q_OBJECT

public:
	WMain(QWidget *parent = 0);
	
	void addLog(const QString &line);

	void addChat(const QString &from, const QString &text);
	void addServerMessage(const QString &text);
	void addServerErrorMessage(int code, const QString &text);

	void updateConnectionStatus();

	QString getUsername() const;

private slots:
	void actionConnect();
	void actionClose();
	void slotSrvTextChanged();
	void actionRegister();
	void actionUnregister();
	void actionSettings();
	void actionAbout();
	
#ifdef DEBUG
	void actionTest();
#endif

private:
	QLineEdit*	editSrvAddr;
	QPushButton*	btnConnect;
	QPushButton*	btnClose;
	
	ChatBox*	m_pChat;
	
	QLineEdit	*editRegister;   // debug, remove later
};

#endif	/* _WMAIN_H */
