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

#include "Config.h"
#include "Debug.h"
#include "ConfigParser.hpp"

#include "pclient.hpp"

#include <cstdio>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

extern ConfigParser config;

WMain::WMain(QWidget *parent) : QWidget(parent)
{
	//setFixedSize(200, 120);
	setWindowTitle(tr("HoldingNuts Foyer"));
	setWindowIcon(QIcon(":/res/pclient.ico"));
	
	QGroupBox *groupInfo = new QGroupBox(tr("User info"));
	
	QLabel *lblUsername = new QLabel(tr("Username:"), this);
	editUsername = new QLineEdit(QString::fromStdString(config.get("player_name")), this);
	
	QGridLayout *lInfo = new QGridLayout();
	lInfo->addWidget(lblUsername, 0, 0);
	lInfo->addWidget(editUsername, 0, 1);
	
	groupInfo->setLayout(lInfo);
	
	////
	
	QGroupBox *groupSrv = new QGroupBox(tr("Connection"));
	
	QLabel *lblHost = new QLabel(tr("Host:"), this);
	
	editSrvAddr = new QLineEdit(QString::fromStdString(config.get("default_host")), this);
	connect(editSrvAddr, SIGNAL(textChanged(const QString&)), this, SLOT(slotSrvTextChanged()));
	
	btnConnect = new QPushButton(tr("Connect"), this);
	btnConnect->setEnabled(false);
	connect(btnConnect, SIGNAL(clicked()), this, SLOT(actionConnect()));
	
	btnClose = new QPushButton(tr("Close"), this);
	btnClose->setEnabled(false);
	connect(btnClose, SIGNAL(clicked()), this, SLOT(actionClose()));
		
	QHBoxLayout *lsrvinp = new QHBoxLayout();
	lsrvinp->addWidget(lblHost);
	lsrvinp->addWidget(editSrvAddr);
	lsrvinp->addWidget(btnConnect);
	lsrvinp->addWidget(btnClose);
	
	editLog = new QTextEdit(this);
	editLog->setReadOnly(true);
	editLog->setFixedHeight(60);
	
	QVBoxLayout *lsrv = new QVBoxLayout();
	lsrv->addLayout(lsrvinp);
	lsrv->addWidget(editLog);
	
	groupSrv->setLayout(lsrv);
	/////////////////////
	
	QPushButton *btnRegister = new QPushButton(tr("Register"), this);
	connect(btnRegister, SIGNAL(clicked()), this, SLOT(actionRegister()));
	
	editRegister = new QLineEdit("0", this);
	editRegister->setFixedWidth(30);
	
	QHBoxLayout *lRegister = new QHBoxLayout();
	lRegister->addWidget(btnRegister);
	lRegister->addWidget(editRegister);
	
	QPushButton *btnTest = new QPushButton(tr("Test"), this);
	connect(btnTest, SIGNAL(clicked()), this, SLOT(actionTest()));
	
	QPushButton *btnSettings = new QPushButton(tr("Settings"), this);
	connect(btnSettings, SIGNAL(clicked()), this, SLOT(actionSettings()));
	
	m_pChat = new ChatBox(tr("Foyer Chat"), ((PClient*)qApp)->getMyCId(), -1);
	
	// final layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(groupInfo);
	layout->addWidget(groupSrv);
	layout->addWidget(m_pChat);
	layout->addLayout(lRegister);
	layout->addWidget(btnSettings);
	layout->addWidget(btnTest);
	
	setLayout(layout);
}

void WMain::addLog(const QString& line)
{
	editLog->append(line);
}

void WMain::addChat(const QString& from, const QString& text)
{
	m_pChat->addMessage(from, text);
}

void WMain::addServerMessage(const QString& text)
{
	m_pChat->addMessage(text, Qt::blue);
}

void WMain::addServerErrorMessage(int code, const QString& text)
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

QString WMain::getUsername() const
{
	return editUsername->text();
}

void WMain::actionConnect()
{
	if (!((PClient*)qApp)->doConnect(editSrvAddr->text(), config.getInt("default_port")))
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
	WTable *table = new WTable(0, 0);
	table->show();
}

void WMain::actionRegister()
{
	const int gid = editRegister->text().toInt();
	((PClient*)qApp)->doRegister(gid);
}

void WMain::actionSettings()
{
	SettingsDialog dialogSettings;
	dialogSettings.exec();
}

void WMain::slotSrvTextChanged()
{
	if (!editSrvAddr->text().length() || ((PClient*)qApp)->isConnected())
		btnConnect->setEnabled(false);
	else
		btnConnect->setEnabled(true);
}
