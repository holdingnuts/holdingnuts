
#include "ChatBox.hpp"
#include "pclient.hpp"

#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

ChatBox::ChatBox(const QString& title, int gid, int tid, QWidget *parent)
:	QGroupBox(title, parent),
	m_nGameID(gid),
	m_nTableID(tid)
{
	m_pEditChat = new QLineEdit(this);
	
	connect(m_pEditChat, SIGNAL(returnPressed()), this, SLOT(actionChat()));
	
	m_pEditChatLog = new QTextEdit(this);
	m_pEditChatLog->setReadOnly(true);
	
	QVBoxLayout *lchat = new QVBoxLayout();

	lchat->addWidget(m_pEditChat);
	lchat->addWidget(m_pEditChatLog);
	
	this->setLayout(lchat);
} // ChatBox

void ChatBox::addMessage(const QString& msg, const QColor& color)
{
	m_pEditChatLog->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	m_pEditChatLog->setTextColor(color);
	m_pEditChatLog->setFontWeight(QFont::Normal);
	m_pEditChatLog->insertPlainText(msg + "\r\n");
	m_pEditChatLog->setTextColor(QColor(0, 0, 0));	
}

void ChatBox::addMessage(const QString& from, const QString& msg)
{
	m_pEditChatLog->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	m_pEditChatLog->setFontWeight(QFont::Bold);
	m_pEditChatLog->insertPlainText("[" + from + "]");
	m_pEditChatLog->setFontWeight(QFont::Normal);
	m_pEditChatLog->insertPlainText(": " + msg + "\r\n");
} // addMessage

void ChatBox::addMessage(const QString& from, const QString& msg, const QColor& color)
{
	m_pEditChatLog->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	m_pEditChatLog->setTextColor(color);
	m_pEditChatLog->setFontWeight(QFont::Bold);
	m_pEditChatLog->insertPlainText("[" + from + "]");
	m_pEditChatLog->setFontWeight(QFont::Normal);
	m_pEditChatLog->insertPlainText(": " + msg + "\r\n");
	m_pEditChatLog->setTextColor(QColor(0, 0, 0));
} // addMessage

void ChatBox::actionChat()
{
	if(m_pEditChat->text().length())
	{
		if(m_nTableID == -1) // foyer chat
			((PClient*)qApp)->chatAll(m_pEditChat->text());
		else
			((PClient*)qApp)->chat(m_pEditChat->text(), m_nGameID, m_nTableID);

		m_pEditChat->clear();
		m_pEditChat->setFocus();
	}
} // actionChat
