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
 *     Michael Miller <michael.miller@holdingnuts.net>
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


#ifndef _HOLDINGNUTS_CHATBOX_H
#define _HOLDINGNUTS_CHATBOX_H

#include <QWidget>

class QLineEdit;
class QTextEdit;
class QPushButton;

class ChatBox : public QWidget
{
	Q_OBJECT

public:
	enum InputLineAlignment
	{
		INPUTLINE_TOP,
		INPUTLINE_BOTTOM
	};

	ChatBox(
		InputLineAlignment align = INPUTLINE_TOP,
		int nTextLogHeight = 0,
		QWidget *parent = 0);
		
	void addMessage(const QString &msg, const QString &from = "", const QColor &color = Qt::black);
	void addMessage(const QString &msg, const QColor &color = Qt::black);
	
	void setFontPointSize(int size);
	int fontPointSize() const;	

	void showChatBtn(bool bShow);
	void showTime(bool bShow);

	void setEnabled(bool enable);

	bool hasInputFocus() const;

protected:
	ChatBox();
	
	void resizeEvent(QResizeEvent *event);

private slots:
	void actionChat();

signals:
     void dispatchedMessage(QString msg);

private:
	int			m_nFontPointSize;
	QLineEdit		*m_pEditChat;
	QTextEdit		*m_pEditChatLog;
	//! \brief send message button, default is hidden
	QPushButton		*m_pSendMsg;
	//! \brief shows human-readable Timestamp 
	bool			m_bShowTime;
};

#endif /* _HOLDINGNUTS_CHATBOX_H */
