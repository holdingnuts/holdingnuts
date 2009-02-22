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
#include "AboutDialog.hpp"

#include "Config.h"
#include "Debug.h"
#include "SysAccess.h"
#include "ConfigParser.hpp"

#include "pclient.hpp"

#include <cstdio>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QMenu>
#include <QMenuBar>


extern ConfigParser config;

WMain::WMain(QWidget *parent) : QMainWindow(parent, 0)
{
	setWindowTitle(tr("HoldingNuts Foyer"));
	setWindowIcon(QIcon(":/res/pclient.ico"));
	
	
	// connection
	QLabel *lblHost = new QLabel(tr("Host:"), this);
	
	editSrvAddr = new QLineEdit(QString::fromStdString(config.get("default_host")), this);
	connect(editSrvAddr, SIGNAL(textChanged(const QString&)), this, SLOT(slotSrvTextChanged()));
	connect(editSrvAddr, SIGNAL(returnPressed()), this, SLOT(actionConnect()));
	
	btnConnect = new QPushButton(tr("&Connect"), this);
	btnConnect->setEnabled(false);
	connect(btnConnect, SIGNAL(clicked()), this, SLOT(actionConnect()));
	
	btnClose = new QPushButton(tr("Cl&ose"), this);
	btnClose->setEnabled(false);
	connect(btnClose, SIGNAL(clicked()), this, SLOT(actionClose()));
		
	QHBoxLayout *lsrvinp = new QHBoxLayout();
	lsrvinp->addWidget(lblHost);
	lsrvinp->addWidget(editSrvAddr);
	lsrvinp->addWidget(btnConnect);
	lsrvinp->addWidget(btnClose);
	
	QGroupBox *groupSrv = new QGroupBox(tr("Connection"), this);
	groupSrv->setLayout(lsrvinp);
	
	
	// the foyer chat box
	m_pChat = new ChatBox(tr("Foyer Chat"), ((PClient*)qApp)->getMyCId(), -1);
	
	
	// game
	QPushButton *btnRegister = new QPushButton(tr("&Register"), this);
	connect(btnRegister, SIGNAL(clicked()), this, SLOT(actionRegister()));
	
	QPushButton *btnUnregister = new QPushButton(tr("&Unregister"), this);
	connect(btnUnregister, SIGNAL(clicked()), this, SLOT(actionUnregister()));
	
	editRegister = new QLineEdit("0", this);
	editRegister->setFixedWidth(30);
	
	QHBoxLayout *lRegister = new QHBoxLayout();
	lRegister->addWidget(btnRegister);
	lRegister->addWidget(btnUnregister);
	lRegister->addWidget(editRegister);
		
	
	// final layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(groupSrv);
	layout->addWidget(m_pChat);
	layout->addLayout(lRegister);
	
	
	// create a main widget containing the layout
	QWidget *central = new QWidget(this);
	central->setLayout(layout);
	setCentralWidget(central);
	
	
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
	
	action = new QAction(tr("Test"), this);
	connect(action, SIGNAL(triggered()), this, SLOT(actionTest()));
	game->addAction(action);
#endif
	
	menuBar()->addMenu(game)->setText(tr("&Game"));
}

void WMain::addLog(const QString &line)
{
	m_pChat->addMessage(line, Qt::darkGreen);
}

void WMain::addChat(const QString &from, const QString &text)
{
	m_pChat->addMessage(from, text);
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

void WMain::updateConnectionStatus()
{
	if (((PClient*)qApp)->isConnected())
	{
		btnConnect->setEnabled(false);
		btnClose->setEnabled(true);
	}
	else
	{
		if (editSrvAddr->text().length())
			btnConnect->setEnabled(true);
		else
			btnConnect->setEnabled(false);
		btnClose->setEnabled(false);
	}
}

void WMain::slotSrvTextChanged()
{
	updateConnectionStatus();
}

QString WMain::getUsername() const
{
	return QString::fromStdString(config.get("player_name"));
}

void WMain::actionConnect()
{
	if (!editSrvAddr->text().length() || ((PClient*)qApp)->isConnected())
		return;
	
	if (!((PClient*)qApp)->doConnect(editSrvAddr->text(), config.getInt("default_port")))
		addLog(tr("Error connecting."));
	else
		btnConnect->setEnabled(false);
}

void WMain::actionClose()
{
	((PClient*)qApp)->doClose();
}

#ifdef DEBUG
void WMain::actionTest()
{
	WTable *table = new WTable(0, 0);
	table->slotShow();
}
#endif

void WMain::actionRegister()
{
	const int gid = editRegister->text().toInt();
	((PClient*)qApp)->doRegister(gid, true);
}

void WMain::actionUnregister()
{
	const int gid = editRegister->text().toInt();
	((PClient*)qApp)->doRegister(gid, false);
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
