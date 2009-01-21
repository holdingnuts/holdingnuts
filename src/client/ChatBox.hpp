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
 */


#ifndef _HOLDING_NUTS_CHAT_BOX_H
#define _HOLDING_NUTS_CHAT_BOX_H

#include <QGroupBox>

class QLineEdit;
class QTextEdit;

class ChatBox : public QGroupBox
{
Q_OBJECT

public:
	ChatBox(
		const QString& title,
		int gid = 0,
		int tid = -1,
		QWidget *parent = 0);
	
	void addMessage(const QString& msg, const QColor& color);
	void addMessage(const QString& from, const QString& msg);
	void addMessage(
		const QString& from,
		const QString& msg,
		const QColor& color);

protected:
	ChatBox();

private slots:
	void actionChat();
	
protected:
	int			m_nGameID;
	int			m_nTableID;
	QLineEdit*		m_pEditChat;
	QTextEdit*		m_pEditChatLog;
};

#endif /* _HOLDING_NUTS_CHAT_BOX_H */
