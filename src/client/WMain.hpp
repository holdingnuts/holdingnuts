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
#include <QItemSelection>

#include <Protocol.h>

class ChatBox;
class GameListTableModel;
class PlayerListTableModel;

class QLineEdit;
class QPushButton;
class QTextEdit;
class QStandardItemModel;
class QTableView;
class QListView;


//! \brief Mainwindow
class WMain : public QMainWindow
{
Q_OBJECT

public:
	WMain(QWidget *parent = 0);
	
	void addLog(const QString &line);

	void addChat(const QString &from, const QString &text);
	void addServerMessage(const QString &text);
	void addServerErrorMessage(int code, const QString &text);

	//! \brief Set the Connect Widgets to right State
	void updateConnectionStatus();

	//! \brief Updates or add a Game to the Gamelist
	//! \param name Gamename
	void updateGamelist(
		int gid,
		const QString& name,
		const QString& type,
		const QString& players,
		const QString& state);
	
	void notifyGameinfo(int gid);
	void notifyGamelist();

	void notifyPlayerinfo(int cid);
	void notifyPlayerlist(int gid);
	
	void updatePlayerList(int gid);


	static QString getGametypeString(gametype type);
	static QString getGamemodeString(gamemode mode);
	static QString getGamestateString(gamestate state);
	
protected:
	void doRegister(bool bRegister);
	
private slots:
	void closeEvent(QCloseEvent *event);
	
	void actionConnect();
	void actionClose();
	
	void slotSrvTextChanged();
	
	void actionRegister();
	void actionUnregister();
	void actionCreateGame();
	
	void actionSettings();
	void actionAbout();
	
	void actionChat(QString msg);
	
	void actionTest();

	void gameListSelectionChanged(
		const QItemSelection& selected,
		const QItemSelection& deselected);
	
	void actionSelectedGameUpdate();

private:
	//! \brief Editbox server adress
	QLineEdit				*editSrvAddr;
	//! \brief Connect Button
	QPushButton				*btnConnect;
	//! \brief Close connection Button
	QPushButton				*btnClose;
	
	//! \Brief Chatbox
	ChatBox					*m_pChat;
	
	//! \brief MVC Model
	GameListTableModel		*modelGameList;
	//! \brief MVC View
	QTableView				*viewGameList;
	
	//! \brief MVC Model
	PlayerListTableModel	*modelPlayerList;
	//! \brief Playerliste from game
	QTableView 	 			*viewPlayerList;
	
	//! \brief create new game
	QPushButton				*btnCreateGame;
	
	//! \brief register to a gamelist
	QPushButton				*btnRegister;
	
	//! \brief un-register a gamelist
	QPushButton				*btnUnregister;
	
	//! \brief timer updates the gamelist
	QTimer		*timerGamelistUpdate;
	
	//! \brief timer updates the selected game
	QTimer		*timerSelectedGameUpdate;
};

#endif	/* _WMAIN_H */
