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


#ifndef _WMAIN_H
#define _WMAIN_H

#include <QWidget>

class ChatBox;
class QLineEdit;
class QPushButton;
class QTextEdit;

class WMain : public QWidget
{
Q_OBJECT

public:
	WMain(QWidget *parent = 0);

	void addLog(const QString& line);

	void addChat(const QString& from, const QString& text);
	void addServerMessage(const QString& text);
	void addServerErrorMessage(int code, const QString& text);

	void updateConnectionStatus();

	QString getUsername() const;

private slots:
	void actionConnect();
	void actionClose();
	void slotSrvTextChanged();
	void actionRegister();
	void actionTest();

private:
	QLineEdit*	editUsername;

	QLineEdit*	editSrvAddr;
	QPushButton*	btnConnect;
	QPushButton*	btnClose;

	QTextEdit*	editLog;

	ChatBox*	m_pChat;
};

#endif	/* _WMAIN_H */
