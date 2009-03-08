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

#include "WMain.hpp"
#include "ChatBox.hpp"
#include "WTable.hpp"
#include "SettingsDialog.hpp"
#include "CreateGameDialog.hpp"
#include "AboutDialog.hpp"
#include "GameListTableModel.hpp"
#include "PlayerListTableModel.hpp"

#include "Config.h"
#include "Debug.h"
#include "SysAccess.h"
#include "ConfigParser.hpp"

#include "pclient.hpp"

#include <cstdio>
#include <vector>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QMenu>
#include <QMenuBar>
#include <QTableView>
#include <QHeaderView>
#include <QListView>
#include <QCloseEvent>

#include "Audio.h"
#include "data.h"

extern ConfigParser config;


WMain::WMain(QWidget *parent) : QMainWindow(parent, 0)
{
	this->setWindowTitle(tr("HoldingNuts Foyer"));
	this->setWindowIcon(QIcon(":/res/pclient.ico"));
	
	
	// header
	QLabel *lblBanner = new QLabel(this);
	lblBanner->setPixmap(QPixmap("gfx/foyer/banner.png"));
	lblBanner->setScaledContents(true);
	lblBanner->setFixedHeight(85);
	
	QPalette p(lblBanner->palette());
	p.setColor(QPalette::Window, Qt::white);
	p.setColor(QPalette::WindowText, Qt::black);
	lblBanner->setPalette(p);
	
	QLabel *lblWelcome = new QLabel("<qt>" + tr("Welcome") +
		" <b>" + QString::fromStdString(config.get("player_name")) + "</b></qt>", this);
	
	QDateTime datetime = QDateTime::currentDateTime();
//	QLabel *lblDateTime = new QLabel(datetime.toString("dddd, yyyy-MM-dd, hh:mm"), this);

	QLabel *lblLogo = new QLabel(this);
	lblLogo->setPixmap(QPixmap(":res/hn_logo.png"));


	QHBoxLayout *lHeader = new QHBoxLayout();
	lHeader->addWidget(lblWelcome, Qt::AlignVCenter);
	lHeader->addWidget(lblLogo);
	lblBanner->setLayout(lHeader);

	// model game
	modelGameList = new GameListTableModel(this);

	// view game
	viewGameList = new QTableView(this);
	viewGameList->setShowGrid(false);
	viewGameList->horizontalHeader()->setHighlightSections(false);
	viewGameList->verticalHeader()->hide();
	viewGameList->verticalHeader()->setHighlightSections(false);
	viewGameList->setSelectionMode(QAbstractItemView::SingleSelection);
	viewGameList->setSelectionBehavior(QAbstractItemView::SelectRows);
	viewGameList->setModel(modelGameList);
	
	connect(
		viewGameList->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this,
		SLOT(gameListSelectionChanged(const QItemSelection&, const QItemSelection&)));

	// model player
	modelPlayerList = new PlayerListTableModel(this);

	// view player
	viewPlayerList = new QTableView(this);
	viewPlayerList->setShowGrid(false);
	viewPlayerList->horizontalHeader()->hide();	// TODO: enable if required
	viewPlayerList->horizontalHeader()->setHighlightSections(false);
	viewPlayerList->verticalHeader()->hide();
	viewPlayerList->verticalHeader()->setHighlightSections(false);
	viewPlayerList->setSelectionMode(QAbstractItemView::SingleSelection);
	viewPlayerList->setSelectionBehavior(QAbstractItemView::SelectRows);
	viewPlayerList->setModel(modelPlayerList);

	// game
	btnRegister = new QPushButton(tr("&Register"), this);
	btnRegister->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnRegister, SIGNAL(clicked()), this, SLOT(actionRegister()));
	
	btnUnregister = new QPushButton(tr("&Unregister"), this);
	btnUnregister->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnUnregister, SIGNAL(clicked()), this, SLOT(actionUnregister()));
	
	QHBoxLayout *lRegister = new QHBoxLayout();
	lRegister->addWidget(btnRegister);
	lRegister->addWidget(btnUnregister);

	// gameinfo
	QVBoxLayout *lGameInfo = new QVBoxLayout();
	lGameInfo->addWidget(new QLabel(tr("Game Name"), this));
	lGameInfo->addWidget(new QLabel(tr("Blinds"), this));
	lGameInfo->addWidget(new QLabel(tr("Starting Blinds"), this));
	lGameInfo->addWidget(new QLabel(tr("Stake"), this));
	lGameInfo->addWidget(new QLabel(tr("Stake"), this));
	lGameInfo->addWidget(viewPlayerList);
	lGameInfo->addLayout(lRegister);

	
	// connection
	editSrvAddr = new QLineEdit(QString::fromStdString(config.get("default_host") + ":" + config.get("default_port")), this);
	connect(editSrvAddr, SIGNAL(textChanged(const QString&)), this, SLOT(slotSrvTextChanged()));
	connect(editSrvAddr, SIGNAL(returnPressed()), this, SLOT(actionConnect()));
	
	btnConnect = new QPushButton(tr("&Connect"), this);
	btnConnect->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnConnect, SIGNAL(clicked()), this, SLOT(actionConnect()));
	
	btnClose = new QPushButton(tr("Cl&ose"), this);
	btnClose->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnClose, SIGNAL(clicked()), this, SLOT(actionClose()));
		
	QHBoxLayout *lConnect = new QHBoxLayout();
	lConnect->addWidget(new QLabel(tr("Server:"), this));
	lConnect->addWidget(editSrvAddr);
	lConnect->addWidget(btnConnect);
	lConnect->addWidget(btnClose);

	
	// the foyer chat box
	m_pChat = new ChatBox("", ChatBox::INPUTLINE_BOTTOM, 0, this);
	m_pChat->showChatBtn(true);
	connect(m_pChat, SIGNAL(dispatchedMessage(QString)), this, SLOT(actionChat(QString)));


	// new game
	btnCreateGame = new QPushButton(tr("Create own game"), this);
	btnCreateGame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnCreateGame, SIGNAL(clicked()), this, SLOT(actionCreateGame()));

	// content layout
	QGridLayout *layout = new QGridLayout;
	// row 0
	layout->addLayout(lConnect, 0, 0, 1, 2);
	// row 2
	layout->addWidget(viewGameList, 1, 0, 1, 1);
	layout->addLayout(lGameInfo, 1, 1, 1, 1);
	// row 3
	layout->addWidget(m_pChat, 2, 0, 1, 1);
	layout->addWidget(btnCreateGame, 2, 1, 1, 1, Qt::AlignBottom | Qt::AlignRight);
	
	layout->setColumnStretch(0, 2);
	layout->setColumnMinimumWidth(0, 300);
	layout->setColumnMinimumWidth(1, 150);
	layout->setContentsMargins(11,11,11,11);  // use a default margin
	
	// the main layout with removed margins
	QVBoxLayout *lMain = new QVBoxLayout;
	lMain->addWidget(lblBanner);
	lMain->addLayout(layout);
	lMain->setContentsMargins(0,0,0,0);
	
	// widget decoration
	QPalette mp(this->palette());
	mp.setColor(QPalette::Window, Qt::black);
	mp.setColor(QPalette::WindowText, Qt::gray);
	//mp.setColor(QPalette::Text, Qt::gray);
	this->setPalette(mp);
	
	
	
	// create a main widget containing the layout
	QWidget *central = new QWidget(this);
	
	central->setLayout(lMain);
	this->setCentralWidget(central);
	
	
	// menus
	QAction *action;
	QMenu *game = new QMenu(this);

	action = new QAction(tr("&Settings"), this);
	action->setShortcut(tr("CTRL+S"));
	connect(action, SIGNAL(triggered()), this, SLOT(actionSettings()));
	game->addAction(action);
	
	game->addSeparator();
	
	action = new QAction(tr("&About"), this);
	action->setShortcut(tr("CTRL+A"));
	connect(action, SIGNAL(triggered()), this, SLOT(actionAbout()));
	game->addAction(action);
	
#ifdef DEBUG
	game->addSeparator();
	
	action = new QAction(tr("&Test"), this);
	action->setShortcut(tr("CTRL+T"));
	connect(action, SIGNAL(triggered()), this, SLOT(actionTest()));
	game->addAction(action);
#endif
	
	game->addSeparator();
	
	action = new QAction(tr("&Quit"), this);
	action->setShortcut(tr("CTRL+Q"));
	connect(action, SIGNAL(triggered()), qApp, SLOT(quit()));
	game->addAction(action);
	
	menuBar()->addMenu(game)->setText(tr("&Game"));
	
	
	// gamelist update timer
	timerGamelistUpdate = new QTimer(this);
	connect(timerGamelistUpdate, SIGNAL(timeout()), qApp, SLOT(requestGamelist()));
	
	// current selected game update timer
	timerSelectedGameUpdate = new QTimer(this);
	connect(timerSelectedGameUpdate, SIGNAL(timeout()), this, SLOT(actionSelectedGameUpdate()));
	
	
	// initially enable/disabled widgets
	updateConnectionStatus();
}

void WMain::addLog(const QString &line)
{
	m_pChat->addMessage(line, Qt::darkGreen);
}

void WMain::addChat(const QString &from, const QString &text)
{
	m_pChat->addMessage(text, from);
}

void WMain::addServerMessage(const QString &text)
{
	m_pChat->addMessage(text, Qt::blue);
}

void WMain::addServerErrorMessage(int code, const QString &text)
{
	QString qmsg = tr("Error") + ": " + text + " (Code: " + QString::number(code) + ")";
	m_pChat->addMessage(qmsg, Qt::red);
}

void WMain::updateGamelist(
	int gid,
	const QString& name,
	const QString& type,
	const QString& players,
	const QString& state)
{
	modelGameList->updateGameName(gid, name);
	modelGameList->updateGameType(gid, type);
	modelGameList->updatePlayers(gid, players);
	modelGameList->updateGameState(gid, state);
	
	viewGameList->resizeColumnsToContents(); 
	viewGameList->resizeRowsToContents(); 
}

void WMain::updateConnectionStatus()
{
	bool is_connected = ((PClient*)qApp)->isConnected();
	
	if (is_connected)
	{
		btnConnect->setEnabled(false);
		btnClose->setEnabled(true);
		
		// update gamelist periodically
		timerGamelistUpdate->start(60*1000);
		
		// update selected game periodically
		timerSelectedGameUpdate->start(15*1000);
	}
	else
	{
		if (editSrvAddr->text().length())
			btnConnect->setEnabled(true);
		else
			btnConnect->setEnabled(false);
		btnClose->setEnabled(false);
		
		modelGameList->clear();
		modelPlayerList->clear();
		
		timerGamelistUpdate->stop();
		timerSelectedGameUpdate->stop();
	}
	
	//m_pChat->setEnabled(is_connected);
	btnCreateGame->setEnabled(is_connected);
	btnRegister->setEnabled(is_connected);
	btnUnregister->setEnabled(is_connected);
}

void WMain::notifyPlayerinfo(int cid)
{
	dbg_msg("DEBUG", "notify playerinfo <%d>", cid);

#if 0
	const playerinfo *player = ((PClient*)qApp)->getPlayerInfo(cid);
	
	if (!player)
		return;
#endif

	// FIXME: update the model only if cid is element of selected game;
	//		and update only the needed items like in gamelistmodel
	//		stringlistmodel should contain <cid> as row.value
	QItemSelectionModel *pSelect = viewGameList->selectionModel();
	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int gid = modelGameList->findGidByRow(pSelect->selectedRows().at(0).row());
		dbg_msg("DEBUG", "select row-id: %d", gid);
		updatePlayerList(gid);
	}
}

void WMain::notifyPlayerlist(int gid)
{
	dbg_msg("DEBUG", "notify playerlist <%d>", gid);
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();
	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int sel_gid = modelGameList->findGidByRow(pSelect->selectedRows().at(0).row());
		
		if (sel_gid == gid)
			updatePlayerList(gid);
	}
}

void WMain::updatePlayerList(int gid)
{
	modelPlayerList->clear();

	const gameinfo *ginfo = ((PClient*)qApp)->getGameInfo(gid);
	
	if (!ginfo)
		return;
	
	for (unsigned int i = 0; i < ginfo->players.size(); ++i)
	{
		const playerinfo *pinfo = ((PClient*)qApp)->getPlayerInfo(ginfo->players[i]);
			
		if (pinfo)
			modelPlayerList->updatePlayerName(i, pinfo->name);
		else
			modelPlayerList->updatePlayerName(i, "???");
	}
	
	// viewPlayerList->resizeColumnsToContents(); // TODO: enable if required 
	viewPlayerList->resizeRowsToContents(); 
}

void WMain::actionConnect()
{
	if (!editSrvAddr->text().length() || ((PClient*)qApp)->isConnected())
		return;
	
	// split up the connect-string: <hostname>:<port>
	QStringList srvlist = editSrvAddr->text().split(":", QString::SkipEmptyParts);
	
	unsigned int port = config.getInt("default_port");
	
	if (!srvlist.count())
		return;
	else if (srvlist.count() > 1)
		port = srvlist.at(1).toInt();
	
	if (!((PClient*)qApp)->doConnect(srvlist.at(0), port))
		addLog(tr("Error connecting."));
	else
		btnConnect->setEnabled(false);
}

void WMain::actionClose()
{
	((PClient*)qApp)->doClose();
}

void WMain::actionTest()
{
#ifdef DEBUG
	WTable *table = new WTable(0, 0);
	table->slotShow();
	
	dbg_msg("DEBUG", "playing test sound");
	audio_play(SOUND_TEST_1);
#endif
}

void WMain::slotSrvTextChanged()
{
	updateConnectionStatus();
}

void WMain::doRegister(bool bRegister)
{
	Q_ASSERT_X(viewGameList, Q_FUNC_INFO, "invalid gamelistview pointer");
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();

	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int gid = modelGameList->findGidByRow(pSelect->selectedRows().at(0).row());
		((PClient*)qApp)->doRegister(gid, bRegister);
		
		updatePlayerList(gid);
	}
}

void WMain::actionRegister()
{
	doRegister(true);
}

void WMain::actionUnregister()
{
	doRegister(false);
}

void WMain::actionSettings()
{
	char cfgfile[1024];
	snprintf(cfgfile, sizeof(cfgfile), "%s/client.cfg", sys_config_path());
	
	SettingsDialog dialogSettings(cfgfile, config);
	dialogSettings.exec();
}

void WMain::actionAbout()
{
	AboutDialog dialogAbout;
	dialogAbout.exec();
}

void WMain::actionCreateGame()
{	
	CreateGameDialog dialogCreateGame;
	if (dialogCreateGame.exec() != QDialog::Accepted)
		return;
	
	QString name = dialogCreateGame.getName();
	unsigned int max_players = dialogCreateGame.getPlayers();
	float stake = dialogCreateGame.getStake();
	((PClient*)qApp)->createGame(name, max_players, stake);
	
	((PClient*)qApp)->requestGamelist();
}

void WMain::actionChat(QString msg)
{
	((PClient*)qApp)->chatAll(msg);
}

void WMain::gameListSelectionChanged(
	const QItemSelection& selected,
	const QItemSelection& deselected)
{
	if (!selected.isEmpty())
	{
		const int selected_row = modelGameList->findGidByRow((*selected.begin()).topLeft().row());
		
		((PClient*)qApp)->requestGameinfo(selected_row);
		((PClient*)qApp)->requestPlayerlist(selected_row);
		
		updatePlayerList(selected_row);
	}
}

void WMain::closeEvent(QCloseEvent *event)
{
	if (((PClient*)qApp)->isTableWindowRemaining())
	{
		event->ignore();
		showMinimized();
	}
	else
		event->accept();
}

QString WMain::getGametypeString(gametype type)
{
	switch (type)
	{
		case GameTypeHoldem:
			return QString(tr("Texas Hold'em"));
		default:
			return QString(tr("unkown gametype"));
	};
}

QString WMain::getGamemodeString(gamemode mode)
{
	switch (mode)
	{
		case GameModeRingGame:
			return QString(tr("Cash game"));
		case GameModeFreezeOut:
			return QString(tr("Tournament"));
		case GameModeSNG:
			return QString(tr("Sit'n'Go"));
		default:
			return QString(tr("unkown gamemode"));
	};
}

QString WMain::getGamestateString(gamestate state)
{
	switch (state)
	{
		case GameStateWaiting:
			return QString(tr("Waiting"));
		case GameStateStarted:
			return QString(tr("Started"));
		case GameStateEnded:
			return QString(tr("Ended"));
		default:
			return QString(tr("unkown gamestate"));
	};
}

void WMain::notifyGameinfo(int gid)
{
	const gameinfo *gi = ((PClient*)qApp)->getGameInfo(gid);
	Q_ASSERT_X(gi, Q_FUNC_INFO, "invalid gameinfo pointer");
	
	updateGamelist(
		gid,
		QString("%1 (%2)").arg(gi->name).arg(gid),
		getGametypeString(gi->type) + " " + getGamemodeString(gi->mode),
		QString("%1 / %2").arg(gi->players_count).arg(gi->players_max),
		getGamestateString(gi->state));
}

void WMain::notifyGamelist()
{
	// FIXME: delete obsolete rows instead of clearing hole list
	modelGameList->clear();
	
	//const gamelist_type &glist = ((PClient*)qApp)->getGameList();
}

void WMain::actionSelectedGameUpdate()
{
	QItemSelectionModel *pSelect = viewGameList->selectionModel();

	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	// only update gameinfo is a game is selected
	if (!pSelect->hasSelection())
		return;
	
	const int gid = modelGameList->findGidByRow(pSelect->selectedRows().at(0).row());
	dbg_msg(Q_FUNC_INFO, "timer: updating game %d", gid);
	
	((PClient*)qApp)->requestGameinfo(gid);
	((PClient*)qApp)->requestPlayerlist(gid);
	
	updatePlayerList(gid);
}
