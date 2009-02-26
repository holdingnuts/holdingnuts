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

class ChatBox;
class StringListModel;

class QLineEdit;
class QPushButton;
class QTextEdit;
class QStandardItemModel;
class QTreeView;
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
	
	void addPlayer(const QString& name);

	//! \brief Updates or add a Game to the Gamelist
	//! \param name Gamename
	void updateGamelist(
		int gid,
		const QString& name,
		const QString& currentPlayers,
		const QString& maxPlayers);

	//! \brief Set the Connect Widgets to right State
	void updateConnectionStatus();

	//! \brief Clears the Modelgamelist
	void clearGamelist();

	//! \brief Clears the Playerlist
	void clearPlayerlist();

	//! \brief Returns the current Username
	//! \return Name
	QString getUsername() const;

private slots:
	void closeEvent(QCloseEvent *event);
	
	void actionConnect();
	void actionClose();
	
	void slotSrvTextChanged();
	
	void actionRegister();
	void actionUnregister();
	
	void actionSettings();
	void actionAbout();
	
	void actionChat(QString msg);
	
#ifdef DEBUG
	void actionTest();
#endif

	void gameListSelectionChanged(
		const QItemSelection& selected,
		const QItemSelection& deselected);

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
	// TODO: replace model with own model class
	QStandardItemModel		*modelGameList;
	//! \brief MVC View
	QTreeView				*viewGameList;
	
	//! \brief MVC Model
	StringListModel			*modelPlayerList;
	//! \brief Playerliste from game
	QListView 	 			*viewPlayerList;
	
	//! \brief create new game
	QPushButton				*btnCreateGame;
};

#endif	/* _WMAIN_H */
