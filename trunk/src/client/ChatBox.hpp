
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
		int				m_nGameID;
		int				m_nTableID;
		QLineEdit*		m_pEditChat;
		QTextEdit*		m_pEditChatLog;
};

#endif	// _HOLDING_NUTS_CHAT_BOX_H
