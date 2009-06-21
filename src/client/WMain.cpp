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


#include "WMain.hpp"
#include "ChatBox.hpp"
#include "WTable.hpp"
#include "SettingsDialog.hpp"
#include "CreateGameDialog.hpp"
#include "AboutDialog.hpp"
#include "GameListTableModel.hpp"
#include "PlayerListTableModel.hpp"
#include "GameListSortFilterProxyModel.hpp"

#include "Config.h"
#include "Debug.h"
#include "SysAccess.h"
#include "Logger.h"
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
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QComboBox>
#include <QStandardItemModel>
#include <QGridLayout>
#include <QCheckBox>

#ifndef NOAUDIO
# include "Audio.h"
#endif
#include "data.h"

extern ConfigParser config;


WMain::WMain(QWidget *parent) : QMainWindow(parent, 0)
{
	this->setWindowTitle(tr("HoldingNuts Foyer"));
	this->setWindowIcon(QIcon(":/res/hn_logo.png"));
	
	
	// header
	QLabel *lblBanner = new QLabel(this);
	lblBanner->setPixmap(QPixmap("gfx/foyer/banner.png"));
	lblBanner->setScaledContents(true);
	lblBanner->setFixedHeight(85);
	
	QPalette p(lblBanner->palette());
	p.setColor(QPalette::Window, Qt::white);
	p.setColor(QPalette::WindowText, Qt::black);
	lblBanner->setPalette(p);
	
	lblWelcome = new QLabel(this);
	connect(lblWelcome, SIGNAL(linkActivated(const QString&)), this, SLOT(actionClose()));
	lblServerTime = new QLabel(this);
	
	QVBoxLayout *lHeaderLabels = new QVBoxLayout;
	lHeaderLabels->addWidget(lblWelcome, 99, Qt::AlignVCenter);
	lHeaderLabels->addWidget(lblServerTime, 1, Qt::AlignBottom);
	
	
	QLabel *lblLogo = new QLabel(this);
	lblLogo->setPixmap(QPixmap(":res/hn_logo_wide.png"));
	
	
	QHBoxLayout *lHeader = new QHBoxLayout;
	lHeader->addLayout(lHeaderLabels);
	lHeader->addWidget(lblLogo, 0, Qt::AlignRight);
	lblBanner->setLayout(lHeader);

	// model game
	modelGameList = new GameListTableModel(this);

	// sort and filter proxy model
	proxyModelGameList = new GameListSortFilterProxyModel(this);
	proxyModelGameList->setSourceModel(modelGameList);
	proxyModelGameList->setDynamicSortFilter(true);
			
	// view game
	viewGameList = new QTableView(this);
	viewGameList->setShowGrid(false);
	viewGameList->setAlternatingRowColors(true);
	viewGameList->horizontalHeader()->setHighlightSections(false);
	viewGameList->verticalHeader()->hide();
	viewGameList->verticalHeader()->setHighlightSections(false);
	viewGameList->setSelectionMode(QAbstractItemView::SingleSelection);
	viewGameList->setSelectionBehavior(QAbstractItemView::SelectRows);
	viewGameList->setSortingEnabled(false);   // FIXME: disabled till issues solved
	
#if 0
	QFont font = viewGameList->font();
	font.setPointSize(font.pointSize() - 2);
	viewGameList->setFont(font);
#endif
	viewGameList->setModel(proxyModelGameList);
	viewGameList->setColumnWidth(0, 200);	// name
	viewGameList->setColumnWidth(1, 90);	// type
	viewGameList->setColumnWidth(2, 50);	// players
	viewGameList->setColumnWidth(3, 50);	// state
	viewGameList->setColumnHidden(4, true);

	connect(
		viewGameList->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		this,
		SLOT(gameListSelectionChanged(const QItemSelection&, const QItemSelection&)));
	
	connect(viewGameList, SIGNAL(activated(const QModelIndex &)), this, SLOT(actionOpenTable()));
	
	// view game filter
	filterPatternGame = new QLineEdit(this);
	filterPatternGame->hide();
	
	chkHideStartedGames = new QCheckBox(tr("hide started"), this);
	connect(chkHideStartedGames, SIGNAL(stateChanged(int)), this, SLOT(filterHideStartedGames(int)));
	
	chkHidePrivateGames = new QCheckBox(tr("hide private"), this);
	connect(chkHidePrivateGames, SIGNAL(stateChanged(int)), this, SLOT(filterHidePrivateGames(int)));

	QHBoxLayout *lGameFilter = new QHBoxLayout();
	lGameFilter->setContentsMargins(0, 0, 0, 0);
	//lGameFilter->addWidget(new QLabel(tr("Game name filter:"), this));
	//lGameFilter->addWidget(filterPatternGame);
	lGameFilter->addWidget(chkHideStartedGames);
	lGameFilter->addWidget(chkHidePrivateGames);

	connect(filterPatternGame,
		SIGNAL(textChanged(const QString&)),
		this,
		SLOT(gameFilterChanged()));

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
	
	// container widget
	wGameFilter = new QWidget(this);
	wGameFilter->setLayout(lGameFilter);
	
	
	// game
	btnRegister = new QPushButton(tr("&Register"), this);
	btnRegister->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnRegister, SIGNAL(clicked()), this, SLOT(actionRegister()));
	
	btnUnregister = new QPushButton(tr("&Unregister"), this);
	btnUnregister->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnUnregister, SIGNAL(clicked()), this, SLOT(actionUnregister()));
	
	btnOpenTable = new QPushButton(tr("&Open table"), this);
	btnOpenTable->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnOpenTable, SIGNAL(clicked()), this, SLOT(actionOpenTable()));
	
	btnStartGame = new QPushButton(tr("&Start game"), this);
	btnStartGame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnStartGame, SIGNAL(clicked()), this, SLOT(actionStartGame()));
	
	QHBoxLayout *lRegister = new QHBoxLayout();
	lRegister->addWidget(btnRegister);
	lRegister->addWidget(btnUnregister);
	
	
	QHBoxLayout *lGameActions = new QHBoxLayout();
	lGameActions->addWidget(btnOpenTable);
	lGameActions->addWidget(btnStartGame);
	
	// gameinfo
	lblGameInfoName = new QLabel(this);
	lblGameInfoPlayers = new QLabel(this);
	lblGameInfoId = new QLabel(this);
	lblGameInfoStakes = new QLabel(this);
	lblGameInfoTimeout = new QLabel(this);
	lblGameInfoBlinds = new QLabel(this);
	
	QFont fntGameInfoTitle = lblGameInfoName->font();

	fntGameInfoTitle.setPointSize(fntGameInfoTitle.pointSize() + 2);
	fntGameInfoTitle.setBold(true); 

	lblGameInfoName->setFont(fntGameInfoTitle);
	lblGameInfoPlayers->setFont(fntGameInfoTitle);
	
	QGridLayout *lGameInfo = new QGridLayout;
	lGameInfo->setContentsMargins(11,11,0,11);
	lGameInfo->addWidget(lblGameInfoName, 0, 0, 1, 1);
	lGameInfo->addWidget(lblGameInfoPlayers, 0, 1, 1, 1, Qt::AlignRight);
	lGameInfo->addWidget(new QLabel(tr("Game ID"), this), 2, 0, 1, 1);
	lGameInfo->addWidget(lblGameInfoId, 2, 1, 1, 1);
	lGameInfo->addWidget(new QLabel(tr("Initial stakes"), this), 3, 0, 1, 1);
	lGameInfo->addWidget(lblGameInfoStakes, 3, 1, 1, 1);
	lGameInfo->addWidget(new QLabel(tr("Player timeout"), this), 4, 0, 1, 1);
	lGameInfo->addWidget(lblGameInfoTimeout, 4, 1, 1, 1);
	lGameInfo->addWidget(new QLabel(tr("Blinds"), this), 5, 0, 1, 1, Qt::AlignTop);
	lGameInfo->addWidget(lblGameInfoBlinds, 5, 1, 1, 1, Qt::AlignTop);
	lGameInfo->addWidget(viewPlayerList, 6, 0, 1, 2);
	lGameInfo->addLayout(lRegister, 7, 0, 1, 2);
	lGameInfo->addLayout(lGameActions, 8, 0, 1, 2);

	wGameInfo = new QWidget(this);
	wGameInfo->setLayout(lGameInfo);
	wGameInfo->setFixedWidth(260);
	
	// connection
	cbSrvAddr = new QComboBox(this);
	cbSrvAddr->setEditable(true); 
	cbSrvAddr->setToolTip(
		tr("The desired server (domain-name or IP) to connect with.\nYou can optionally specify a port number.\nFormat: <host>[:<port>]"));
	connect(cbSrvAddr->lineEdit(), SIGNAL(returnPressed()), this, SLOT(actionConnect()));
	
	btnConnect = new QPushButton(tr("&Connect"), this);
	btnConnect->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnConnect, SIGNAL(clicked()), this, SLOT(actionConnect()));
	
	btnClose = new QPushButton(tr("Cl&ose"), this);
	btnClose->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnClose, SIGNAL(clicked()), this, SLOT(actionClose()));

	QLabel *lblServer = new QLabel(tr("Server:"), this);
	lblServer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	QHBoxLayout *lConnect = new QHBoxLayout();
	lConnect->setContentsMargins(0,0,0,0);
	lConnect->addWidget(lblServer);
	lConnect->addWidget(cbSrvAddr);
	lConnect->addWidget(btnConnect);
	lConnect->addWidget(btnClose);
	
	// container widget
	wConnection = new QWidget(this);
	wConnection->setLayout(lConnect);
	
	
	// the foyer chat box
	m_pChat = new ChatBox(ChatBox::INPUTLINE_BOTTOM, 0, this);
	m_pChat->showChatBtn(true);
	m_pChat->showTime((config.getInt("chat_verbosity_foyer") & 0x1));
	connect(m_pChat, SIGNAL(dispatchedMessage(QString)), this, SLOT(actionChat(QString)));


	// new game
	btnCreateGame = new QPushButton(tr("Create own game"), this);
	btnCreateGame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(btnCreateGame, SIGNAL(clicked()), this, SLOT(actionCreateGame()));

	// content layout
	QGridLayout *layout = new QGridLayout;
	// row 0
	layout->addWidget(wConnection, 0, 0, 1, 2);
	// row 1
	layout->addWidget(viewGameList, 1, 0, 1, 1);
	layout->addWidget(wGameInfo, 1, 1, 2, 1, Qt::AlignCenter | Qt::AlignTop);
	// row 2
	layout->addWidget(wGameFilter, 2, 0, 1, 1);
	// row 3
	layout->addWidget(m_pChat, 3, 0, 1, 1);
	layout->addWidget(btnCreateGame, 3, 1, 1, 1, Qt::AlignBottom | Qt::AlignRight);
	
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
#if 0	
	QPalette mp(this->palette());
	mp.setColor(QPalette::Window, Qt::black);
	mp.setColor(QPalette::WindowText, Qt::gray);
	//mp.setColor(QPalette::Text, Qt::gray);
	this->setPalette(mp);
#endif	
	
	
	// create a main widget containing the layout
	QWidget *central = new QWidget(this);
	
	central->setLayout(lMain);
	this->setCentralWidget(central);
	
	
	// menus
	QAction *action;
	QMenu *menuGame = new QMenu(this);

	action = new QAction(tr("&Settings"), this);
	action->setShortcut(tr("CTRL+S"));
	connect(action, SIGNAL(triggered()), this, SLOT(actionSettings()));
	menuGame->addAction(action);
	
#ifdef DEBUG
	menuGame->addSeparator();
	
	action = new QAction(tr("&Test"), this);
	action->setShortcut(tr("CTRL+T"));
	connect(action, SIGNAL(triggered()), this, SLOT(actionTest()));
	menuGame->addAction(action);
#endif
	
	menuGame->addSeparator();
	
	action = new QAction(tr("&Quit"), this);
	action->setShortcut(tr("CTRL+Q"));
	connect(action, SIGNAL(triggered()), qApp, SLOT(quit()));
	menuGame->addAction(action);
	
	// help menu
	QMenu *menuHelp = new QMenu(this);
	
	action = new QAction(tr("&Handbook"), this);
	action->setShortcut(tr("CTRL+H"));
	connect(action, SIGNAL(triggered()), this, SLOT(actionHelp()));
	menuHelp->addAction(action);
	
	menuHelp->addSeparator();
	
	action = new QAction(tr("&About"), this);
	action->setShortcut(tr("CTRL+A"));
	connect(action, SIGNAL(triggered()), this, SLOT(actionAbout()));
	menuHelp->addAction(action);
	
	// add menus to main-window menu-bar
	menuBar()->addMenu(menuGame)->setText(tr("&Game"));
	menuBar()->addMenu(menuHelp)->setText(tr("&Help"));
	
	
	// gamelist update timer
	timerGamelistUpdate = new QTimer(this);
	connect(timerGamelistUpdate, SIGNAL(timeout()), qApp, SLOT(requestGamelist()));
	
	// current selected game update timer
	timerSelectedGameUpdate = new QTimer(this);
	connect(timerSelectedGameUpdate, SIGNAL(timeout()), this, SLOT(actionSelectedGameUpdate()));
	
	// current selected game update timer
	timerServerTimeUpdate= new QTimer(this);
	connect(timerServerTimeUpdate, SIGNAL(timeout()), this, SLOT(updateServerTimeLabel()));
	
	
	// initially enable/disabled widgets
	updateConnectionStatus();

	// load gui settings
	QSettings settings;

	settings.beginGroup("MainWindow");
		this->resize(settings.value("size", QSize(700, 600)).toSize());
		this->move(settings.value("pos", QPoint(50, 50)).toPoint());
		// filter
		chkHideStartedGames->setCheckState(
			Qt::CheckState(settings.value("filterHideStartedGames", 0).toInt()));
		chkHidePrivateGames->setCheckState(
			Qt::CheckState(settings.value("filterHidePrivateGames", 0).toInt()));
	settings.endGroup();
	
	// load serverlist model
	QStandardItemModel *modelSrvLst = new QStandardItemModel(0, 2, parent);

	modelSrvLst->setHeaderData(0, Qt::Horizontal, tr("Name"));
	modelSrvLst->setHeaderData(1, Qt::Horizontal, tr("Connections"));
	
	int nSrvAddr = settings.beginReadArray("Serverlist");
	
	for (int i = 0; i < nSrvAddr; ++i)
	{
		settings.setArrayIndex(i);
		
		QString server = settings.value("Name").toString();

		if (!server.isEmpty())
		{
			modelSrvLst->insertRow(0);
			modelSrvLst->setData(modelSrvLst->index(0, 0), server);
			modelSrvLst->setData(modelSrvLst->index(0, 1), settings.value("Connections").toInt());
		}
	}
	settings.endArray();

	modelSrvLst->sort(1, Qt::DescendingOrder);

	// no serverhistory found then use default server from config
	if (modelSrvLst->rowCount() == 0)
	{
		modelSrvLst->insertRow(0);
		modelSrvLst->setData(
			modelSrvLst->index(0, 0),
			QString::fromStdString(config.get("default_host") + ":" + config.get("default_port")));
		modelSrvLst->setData(modelSrvLst->index(0, 1), 0);
	}
		
	cbSrvAddr->setModel(modelSrvLst);
}

void WMain::addLog(const QString &line)
{
	m_pChat->addMessage(line, Qt::darkGreen);
}

void WMain::addChat(const QString &from, const QString &text)
{
	m_pChat->addMessage(text, from);
	
	if (config.getBool("log_chat"))
		log_msg("foyer", "(%s) %s",
			 from.toStdString().c_str(), text.toStdString().c_str());
}

void WMain::addServerMessage(const QString &text)
{
	m_pChat->addMessage(text, Qt::blue);
	
	log_msg("infomsg", "%s", text.toStdString().c_str());
}

void WMain::addServerErrorMessage(int code, const QString &text)
{
	QString qmsg = tr("Error") + ": " + text + " (Code: " + QString::number(code) + ")";
	m_pChat->addMessage(qmsg, Qt::red);
	
	log_msg("errmsg", "Error: %s (Code: %d)", text.toStdString().c_str(), code);
}

void WMain::updateConnectionStatus()
{
	const bool is_connected = ((PClient*)qApp)->isConnected();
	const bool is_connecting = ((PClient*)qApp)->isConnecting();
	
	if (is_connecting)
	{
		btnConnect->setEnabled(false);
		btnClose->setEnabled(true);
		cbSrvAddr->setEnabled(false);
		
		wConnection->setVisible(true);
	}
	else if (is_connected)
	{
		btnConnect->setEnabled(false);
		btnClose->setEnabled(true);
		cbSrvAddr->setEnabled(false);
		
		// setup timers for gamelist update, select-game update, server-time update
		timerGamelistUpdate->start(60*1000);
		timerSelectedGameUpdate->start(15*1000);
		timerServerTimeUpdate->start(1000);
		
		wConnection->setVisible(false);
	}
	else  // disconnected
	{
		if (cbSrvAddr->lineEdit()->text().length())
			btnConnect->setEnabled(true);
		else
			btnConnect->setEnabled(false);
		btnClose->setEnabled(false);
		cbSrvAddr->setEnabled(true);
		
		modelGameList->clear();
		modelPlayerList->clear();
		
		timerGamelistUpdate->stop();
		timerSelectedGameUpdate->stop();
		timerServerTimeUpdate->stop();
		
		wConnection->setVisible(true);
	}
	
	m_pChat->setEnabled(is_connected);
	btnCreateGame->setEnabled(is_connected);
	viewGameList->setEnabled(is_connected);
	viewPlayerList->setEnabled(is_connected);
	wGameFilter->setEnabled(is_connected);
	wGameInfo->setEnabled(is_connected);
	
	updateWelcomeLabel();
	updateServerTimeLabel();
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();
	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int selected_row = proxyModelGameList->mapToSource(pSelect->selectedRows().at(0)).row();
		const int gid = modelGameList->findGidByRow(selected_row);
		
		updateGameinfo(gid);
	}
	else
		updateGameinfo(-1);
}

void WMain::notifyPlayerinfo(int cid)
{
	//dbg_msg("DEBUG", "notify playerinfo <%d>", cid);

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
		const int selected_row = proxyModelGameList->mapToSource(pSelect->selectedRows().at(0)).row();
		const int gid = modelGameList->findGidByRow(selected_row);
		//dbg_msg("DEBUG", "select-row:%d gid:%d", selected_row, gid);
		updatePlayerList(gid);
	}
}

void WMain::notifyPlayerlist(int gid)
{
	//dbg_msg("DEBUG", "notify playerlist <%d>", gid);
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();
	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int selected_row = proxyModelGameList->mapToSource(pSelect->selectedRows().at(0)).row();
		const int sel_gid = modelGameList->findGidByRow(selected_row);
		
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
			modelPlayerList->updatePlayerName(i,
				QString("??? (%1)").arg(ginfo->players[i]));
	}
	
	viewPlayerList->resizeRowsToContents(); 
}

void WMain::actionConnect()
{
	if (!cbSrvAddr->lineEdit()->text().length() || ((PClient*)qApp)->isConnected())
		return;
	
	// store server in combobox and settings
	int idx = cbSrvAddr->findText(cbSrvAddr->lineEdit()->text());

	QAbstractItemModel *modelSrvLst = cbSrvAddr->model();

	if (idx == -1)
	{
		modelSrvLst->insertRow(0);
		modelSrvLst->setData(modelSrvLst->index(0, 0), cbSrvAddr->lineEdit()->text());
		modelSrvLst->setData(modelSrvLst->index(0, 1), 1);
	}
	else
	{
		int count = modelSrvLst->data(modelSrvLst->index(idx, 1)).toInt();

		modelSrvLst->setData(modelSrvLst->index(idx, 1), ++count);
		modelSrvLst->sort(1, Qt::DescendingOrder);
	}
	
	writeServerlist();
	
	// split up the connect-string: <hostname>:<port>
	QStringList srvlist = cbSrvAddr->lineEdit()->text().split(":", QString::SkipEmptyParts);
	
	unsigned int port = config.getInt("default_port");
	
	if (!srvlist.count())
		return;
	else if (srvlist.count() > 1)
		port = srvlist.at(1).toInt();
	
	if (!((PClient*)qApp)->doConnect(srvlist.at(0), port))
		addLog(tr("Error connecting."));
	else
	{
		updateConnectionStatus();
		
		// FIXME: set focus on chat input line
		//m_pChat->setFocus(Qt::OtherFocusReason);
	}
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
	table->showDebugTable();
	
#ifndef NOAUDIO
	dbg_msg("DEBUG", "playing test sound");
	audio_play(SOUND_TEST_1);
#endif /* NOAUDIO */
#endif
}

void WMain::doRegister(bool bRegister)
{
	Q_ASSERT_X(viewGameList, Q_FUNC_INFO, "invalid gamelistview pointer");
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();

	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int selected_row = proxyModelGameList->mapToSource(pSelect->selectedRows().at(0)).row();
		const int gid = modelGameList->findGidByRow(selected_row);
		
		const gameinfo *gi = ((PClient*)qApp)->getGameInfo(gid);
		Q_ASSERT_X(gi, Q_FUNC_INFO, "invalid gameinfo pointer");
		
		QString password;
		if (bRegister && gi->password)
		{
			bool ok;
			password = QInputDialog::getText(this, tr("Private game"),
					tr("Please enter the game password:"), (config.getBool("ui_echo_password") ? QLineEdit::Normal : QLineEdit::Password), //inline if statement to echo or mask password
						QString(), &ok);
			if (!ok)
				return;
		}
		
		((PClient*)qApp)->doRegister(gid, bRegister, password);
		((PClient*)qApp)->requestGameinfo(gid);
		((PClient*)qApp)->requestPlayerlist(gid);
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

void WMain::actionOpenTable()
{
	Q_ASSERT_X(viewGameList, Q_FUNC_INFO, "invalid gamelistview pointer");
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();

	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int selected_row = proxyModelGameList->mapToSource(pSelect->selectedRows().at(0)).row();
		const int gid = modelGameList->findGidByRow(selected_row);
		
		tableinfo* tinfo = ((PClient*)qApp)->getTableInfo(gid, 0 /* FIXME */);
		
		// drop message and display notice if there is no table-info
		if (!tinfo)
		{
			addLog(tr("The table will be opened on the next hand."));
			return;
		}
		
		tinfo->window->show();
	}
}

void WMain::actionStartGame()
{
	Q_ASSERT_X(viewGameList, Q_FUNC_INFO, "invalid gamelistview pointer");
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();

	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int selected_row = proxyModelGameList->mapToSource(pSelect->selectedRows().at(0)).row();
		const int gid = modelGameList->findGidByRow(selected_row);
		
		((PClient*)qApp)->doStartGame(gid);
		((PClient*)qApp)->requestGameinfo(gid);
	}
}

void WMain::actionSettings()
{
	char cfgfile[1024];
	snprintf(cfgfile, sizeof(cfgfile), "%s/client.cfg", sys_config_path());
	
	SettingsDialog dialogSettings(config);
	if (dialogSettings.exec() != QDialog::Accepted)
		return;
	
	// save the settings
	config.save(cfgfile);
	
	// updates
	this->updateWelcomeLabel();
	
	addLog(tr("You may need to restart the client for all settings to take effect."));
}

void WMain::actionHelp()
{
	QDesktopServices::openUrl(QUrl(CLIENT_WEBSITE_HELP));
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
	
	gamecreate gc;
	
	gc.name = dialogCreateGame.getName();
	gc.max_players = dialogCreateGame.getPlayers();
	gc.stake = dialogCreateGame.getStake();
	gc.timeout = dialogCreateGame.getTimeout();
	gc.blinds_start = dialogCreateGame.getBlindsStart();
	gc.blinds_factor = dialogCreateGame.getBlindsFactor();
	gc.blinds_time = dialogCreateGame.getBlindsTime();
	gc.password = dialogCreateGame.getPassword();

	((PClient*)qApp)->createGame(&gc);
	((PClient*)qApp)->requestGamelist();
}

void WMain::actionChat(QString msg)
{
	// send raw to server if first char is '/'
	if (config.getBool("chat_console") && msg.at(0) == QChar('/'))
	{
		((PClient*)qApp)->sendDebugMsg(msg.mid(1));
		return;
	}

	((PClient*)qApp)->chatAll(msg);
}

void WMain::gameListSelectionChanged(
	const QItemSelection& selected,
	const QItemSelection& deselected)
{
	if (!selected.isEmpty())
	{
		const int selected_row = proxyModelGameList->mapToSource((*selected.begin()).topLeft()).row();
		const int gid = modelGameList->findGidByRow(selected_row);
		
		/*
		dbg_msg("gameListSelectionChanged", "selected-row:%d gid:%d",
			selected_row,
			gid);
		*/
		
		((PClient*)qApp)->requestGameinfo(gid);
		((PClient*)qApp)->requestPlayerlist(gid);
		
		updateGameinfo(gid);
	}
}

void WMain::gameFilterChanged()
{
	proxyModelGameList->setFilterRegExp(
		QRegExp(filterPatternGame->text(), Qt::CaseInsensitive));
}

void WMain::closeEvent(QCloseEvent *event)
{
	if (((PClient*)qApp)->isTableWindowRemaining())
	{
		event->ignore();
		showMinimized();
	}
	else
	{
		// store settings
		QSettings settings;

		settings.beginGroup("MainWindow");
			settings.setValue("size", this->size());
			settings.setValue("pos", this->pos());
			// filter
			settings.setValue("filterHideStartedGames", chkHideStartedGames->checkState());
			settings.setValue("filterHidePrivateGames", chkHidePrivateGames->checkState());
		settings.endGroup();

		event->accept();
	}
}

QString WMain::getGametypeString(gametype type)
{
	switch (type)
	{
		case GameTypeHoldem:
			return QString(tr("THNL"));
		default:
			return QString(tr("unkown gametype"));
	}
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
	}
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
	}
}

void WMain::notifyGameinfo(int gid)
{
	const gameinfo *gi = ((PClient*)qApp)->getGameInfo(gid);
	Q_ASSERT_X(gi, Q_FUNC_INFO, "invalid gameinfo pointer");
		
	modelGameList->updateGameName(gid, gi->name);
	modelGameList->updateGameType(gid, getGametypeString(gi->type) + " " + getGamemodeString(gi->mode));
	modelGameList->updatePlayers(gid, QString("%1 / %2").arg(gi->players_count).arg(gi->players_max));
	modelGameList->updateGameState(gid, getGamestateString(gi->state));
	modelGameList->updatePassword(gid, gi->password);
	
	viewGameList->resizeRowsToContents();
	
	// update gameinfo panel
	Q_ASSERT_X(viewGameList, Q_FUNC_INFO, "invalid gamelistview pointer");
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();
	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	if (pSelect->hasSelection())
	{
		const int selected_row = proxyModelGameList->mapToSource(pSelect->selectedRows().at(0)).row();
		const int sel_gid = modelGameList->findGidByRow(selected_row);
		
		if (gid == sel_gid)
			updateGameinfo(gid);
	}
}

void WMain::notifyGamelist()
{
	// remove all vanished items from modelGameList
	for (int i=0; i < modelGameList->rowCount(); i++)
	{
		const int gid = modelGameList->findGidByRow(i);
		
		// remove abaondoned games from list
		if (!((PClient*)qApp)->isGameInList(gid))
			modelGameList->removeRow(i);
	}
}

void WMain::actionSelectedGameUpdate()
{
#if 0
	// do not update if window hasn't the focus
	if (!isActiveWindow())
		return;
#endif
	
	
	QItemSelectionModel *pSelect = viewGameList->selectionModel();

	Q_ASSERT_X(pSelect, Q_FUNC_INFO, "invalid selection model pointer");
	
	// only update gameinfo is a game is selected
	if (!pSelect->hasSelection())
		return;
	
	const int selected_row = proxyModelGameList->mapToSource(pSelect->selectedRows().at(0)).row();
	const int gid = modelGameList->findGidByRow(selected_row);
	
	//dbg_msg(Q_FUNC_INFO, "timer: updating game %d", gid);
	
	((PClient*)qApp)->requestGameinfo(gid);
	((PClient*)qApp)->requestPlayerlist(gid);
}

void WMain::updateWelcomeLabel()
{
	const bool is_connected = ((PClient*)qApp)->isConnected();
	
	QString strConnectionState;
	if (is_connected)
		strConnectionState = "<a href=\"#logout\">" + tr("Logout") + "</a>";
	else
		strConnectionState = "offline";
	
	lblWelcome->setText("<qt>" + tr("Welcome") +
		" <strong>" + QString::fromStdString(config.get("player_name")) + "</strong>" +
		"&nbsp; <em>(" + strConnectionState + ")</em>" +
		"</qt>");
}

void WMain::updateGameinfo(int gid)
{
	if (gid == -1)
	{
		lblGameInfoName->clear();
		lblGameInfoPlayers->clear();
		lblGameInfoId->clear();
		lblGameInfoStakes->clear();
		lblGameInfoTimeout->clear();
		lblGameInfoBlinds->clear();
		
		btnRegister->setEnabled(false);
		btnUnregister->setEnabled(false);
		
		btnOpenTable->setVisible(false);
		btnStartGame->setVisible(false);
		
		return;
	}
	
	const gameinfo *gi = ((PClient*)qApp)->getGameInfo(gid);
	
	if (!gi)
		return;
	
	lblGameInfoName->setText(gi->name);
	lblGameInfoPlayers->setText(QString("%1 / %2").arg(gi->players_count).arg(gi->players_max));
	lblGameInfoId->setText(QString("%1").arg(gid));
	lblGameInfoStakes->setText(QString("%1").arg(gi->initial_stakes));
	lblGameInfoTimeout->setText(QString("<qt>%1<b>s</b></qt>").arg(gi->player_timeout));
	lblGameInfoBlinds->setText(QString("<qt>%1 /<br/><b>x</b>%2 /<br/>%3<b>s</b></qt>")
		.arg(gi->blinds_start)
		.arg(gi->blinds_factor, 0, 'f', 2)
		.arg(gi->blinds_time));
	
	
	
	btnRegister->setEnabled(!gi->registered && gi->state == GameStateWaiting);
	btnUnregister->setEnabled(gi->registered && gi->state == GameStateWaiting);
	
	btnOpenTable->setVisible(gi->registered && gi->state != GameStateWaiting);
	btnStartGame->setVisible(gi->registered &&
		gi->owner &&
		gi->players_count >= 2 &&
		gi->state == GameStateWaiting);
}

void WMain::updateServerTimeLabel()
{
	const bool is_connected = ((PClient*)qApp)->isConnected();
	
	if (is_connected)
	{
		const QDateTime timeServer = ((PClient*)qApp)->getServerTime();
		lblServerTime->setText(timeServer.toString("dddd, yyyy-MM-dd, hh:mm:ss"));
	}
	else
		lblServerTime->clear();
}

void WMain::writeServerlist() const
{
	QSettings settings;

	settings.beginWriteArray("Serverlist");
	
	int numServer = cbSrvAddr->count();
	
	// store maximal 10 server in recent list
	if (numServer > 10)
		numServer = 10;

	QAbstractItemModel *pModel = cbSrvAddr->model();

	for (int i = 0; i < numServer; ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("Name", pModel->data(pModel->index(i, 0)));
		settings.setValue("Connections", pModel->data(pModel->index(i, 1)).toInt());
	}

	settings.endArray();
}

void WMain::filterHideStartedGames(int state)
{
	if (state == Qt::Checked)
		proxyModelGameList->hideGameState(getGamestateString(GameStateStarted));
	else		
		proxyModelGameList->showGameState(getGamestateString(GameStateStarted));
}

void WMain::filterHidePrivateGames(int state)
{
	proxyModelGameList->showPrivateGames(state == Qt::Checked);
}
