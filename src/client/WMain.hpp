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
 *     Michael Miller <michael.miller@holdingnuts.net>
 */


#ifndef _WMAIN_H
#define _WMAIN_H

#include <QMainWindow>
#include <QItemSelection>

#include <Protocol.h>

class ChatBox;
class GameListTableModel;
class PlayerListTableModel;
class GameListSortFilterProxyModel;

class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QStandardItemModel;
class QTableView;
class QListView;
class QComboBox;
class QCheckBox;

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
	
	void updateWelcomeLabel();
	void updateGameinfo(int gid);
	
	void writeServerlist() const;

private slots:
	void closeEvent(QCloseEvent *event);
	
	void actionConnect();
	void actionClose();
	
	void actionRegister();
	void actionUnregister();
	void actionOpenTable();
	void actionCreateGame();
	void actionStartGame();
	
	void actionSettings();
	
	void actionHelp();
	void actionAbout();
	
	void actionChat(QString msg);
	
	void actionTest();

	void gameListSelectionChanged(
		const QItemSelection& selected,
		const QItemSelection& deselected);
	
	void actionSelectedGameUpdate();
	
	void gameFilterChanged();
	
	void updateServerTimeLabel();
	
	void filterHideStartedGames(int state);
	void filterHidePrivateGames(int state);
	
private:
	//! \brief Label in header displaying a welcome message
	QLabel			*lblWelcome;
	//! \brief Label in header displaying the server time
	QLabel 			*lblServerTime;
	
	//! \brief Combobox server adress
	QComboBox		*cbSrvAddr;
	//! \brief Connect Button
	QPushButton		*btnConnect;
	//! \brief Close connection Button
	QPushButton		*btnClose;
	//! \brief Container widget for connection-widgets
	QWidget			*wConnection;
	
	//! \Brief Chatbox
	ChatBox			*m_pChat;
	
	//! \brief MVC Model
	GameListTableModel	*modelGameList;
	//! \brief MVC View
	QTableView		*viewGameList;
	//! \brief Sort- and Filter Proxy Model
	GameListSortFilterProxyModel	*proxyModelGameList;
	
	QLineEdit		*filterPatternGame;
	//! \brief Checkbox for hiding started games
	QCheckBox		*chkHideStartedGames;
	//! \brief Checkbox for hiding private games
	QCheckBox		*chkHidePrivateGames;
	
	//! \brief MVC Model
	PlayerListTableModel	*modelPlayerList;
	//! \brief Playerlist of game
	QTableView 	 	*viewPlayerList;
	
	//! \brief Label for Gamename
	QLabel			*lblGameInfoName;
	//! \brief Label for the number of Players in selected Game
	QLabel			*lblGameInfoPlayers;
	//! \brief Label for Game Id
	QLabel			*lblGameInfoId;
	//! \brief Label for initial player stakes
	QLabel			*lblGameInfoStakes;
	//! \brief Label for player timeout
	QLabel			*lblGameInfoTimeout;
	//! \brief Label for blinds settings
	QLabel			*lblGameInfoBlinds;
	
	//! \brief Container widget for gamelist filter widgets
	QWidget			*wGameFilter;
	//! \brief Container widget for gameinfo widgets
	QWidget			*wGameInfo;
	
	//! \brief Create new game
	QPushButton		*btnCreateGame;
	
	//! \brief Register to a gamelist
	QPushButton		*btnRegister;
	
	//! \brief Un-register a gamelist
	QPushButton		*btnUnregister;
	
	QPushButton		*btnOpenTable;
	QPushButton		*btnStartGame;
	
	//! \brief Timer for updating the gamelist
	QTimer			*timerGamelistUpdate;
	
	//! \brief Timer for updating the selected game
	QTimer			*timerSelectedGameUpdate;
	
	//! \brief Timer for updating the server timerGamelistUpdate
	QTimer			*timerServerTimeUpdate;
};

#endif	/* _WMAIN_H */
