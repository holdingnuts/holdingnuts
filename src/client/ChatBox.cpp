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


#include "ChatBox.hpp"

#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QPushButton>
#include <QTime>

ChatBox::ChatBox(
	InputLineAlignment align,
	int nTextLogHeight,
	QWidget *parent) : QWidget(parent), m_bShowTime(false)
{
	m_pEditChat = new QLineEdit(this);
	m_pEditChat->setMaxLength(200);

	connect(m_pEditChat, SIGNAL(returnPressed()), this, SLOT(actionChat()));

	m_pEditChatLog = new QTextEdit(this);
	m_pEditChatLog->setReadOnly(true);
	
	if(nTextLogHeight > 0)
		m_pEditChatLog->setFixedHeight(nTextLogHeight);

	m_pSendMsg = new QPushButton(tr("Chat"), this);
	m_pSendMsg->setVisible(false);

	connect(m_pSendMsg, SIGNAL(clicked()), this, SLOT(actionChat()));

	// layout
	QHBoxLayout *lInputLine = new QHBoxLayout;
	
	lInputLine->addWidget(m_pEditChat);
	lInputLine->addWidget(m_pSendMsg);

	QVBoxLayout *lchat = new QVBoxLayout(this);

	
	if (align == INPUTLINE_TOP)
		lchat->addLayout(lInputLine);

	lchat->addWidget(m_pEditChatLog);

	if (align == INPUTLINE_BOTTOM)
		lchat->addLayout(lInputLine);

	// qwidget has already margins
	lchat->setContentsMargins(0, 0, 0, 0);

	setLayout(lchat);

	m_nFontPointSize = this->fontPointSize();
}

void ChatBox::addMessage(const QString &msg, const QString &from, const QColor &color)
{
	// save current scroll-position
	QScrollBar *sb = m_pEditChatLog->verticalScrollBar();
	int scrollpos = sb->value();
	bool was_bottom = (sb->value() == sb->maximum());
	
	// save current cursor
	QTextCursor c = m_pEditChatLog->textCursor();
	
	// move cursor position to end
	m_pEditChatLog->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	int new_pos = m_pEditChatLog->textCursor().position();
	
	// set format at current cursor position
	m_pEditChatLog->setTextColor(color);
	m_pEditChatLog->setFontPointSize(m_nFontPointSize);
	
	if (m_bShowTime)
		m_pEditChatLog->insertPlainText("<" + QTime::currentTime().toString("hh:mm") + "> ");
	
	// is the message from other client
	if (from.length())
	{
		m_pEditChatLog->setFontWeight(QFont::Bold);
		m_pEditChatLog->insertPlainText("[" + from + "] ");
	}
	
	m_pEditChatLog->setFontWeight(QFont::Normal);
	m_pEditChatLog->insertPlainText(msg + "\r\n");
	
	// restore previous cursor position and selection
	if (new_pos != c.position())
		m_pEditChatLog->setTextCursor(c);
	
	// was the scroll-position at bottom?
	if (was_bottom)
	{
		// the the new scroll-position to bottom in order to display multilines completely
		sb->setValue(sb->maximum());
	}
	else
	{
		// restore old position
		sb->setValue(scrollpos);
	}
}

void ChatBox::addMessage(const QString &msg, const QColor &color)
{
	addMessage(msg, "", color);
}

void ChatBox::setFontPointSize(int size)
{
	m_nFontPointSize = size;
}

int ChatBox::fontPointSize() const
{
	return m_pEditChatLog->currentFont().pointSize();
}

void ChatBox::showChatBtn(bool bShow)
{
	m_pSendMsg->setVisible(bShow);
}

void ChatBox::showTime(bool bShow)
{
	m_bShowTime = bShow;
}

void ChatBox::setEnabled(bool enable)
{
	m_pEditChat->setEnabled(enable);
	m_pSendMsg->setEnabled(enable);
}

bool ChatBox::hasInputFocus() const
{
	Q_ASSERT_X(m_pEditChat, Q_FUNC_INFO, "invalid lineedit pointer");

	return m_pEditChat->hasFocus();
}

void ChatBox::resizeEvent(QResizeEvent *event)
{
	// scroll the chatlog to bottom
	QScrollBar *sb = m_pEditChatLog->verticalScrollBar();
	sb->setValue(sb->maximum());
}

void ChatBox::actionChat()
{
	if (m_pEditChat->text().length())
	{
		emit dispatchedMessage(m_pEditChat->text());
		
		m_pEditChat->clear();
		m_pEditChat->setFocus();
	}
}
