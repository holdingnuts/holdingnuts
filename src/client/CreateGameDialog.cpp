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


#include <QtGui>

#include "Debug.h"
#include "ConfigParser.hpp"
#include "CreateGameDialog.hpp"

extern ConfigParser config;

CreateGameDialog::CreateGameDialog(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(tr("Create game"));
	setWindowIcon(QIcon(":/res/hn_logo.png"));
	setMinimumWidth(300);
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
					  Qt::Horizontal, this);
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(actionOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	
	QGroupBox *groupGeneral = new QGroupBox(tr("General"), this);
	
	QLabel *labelName = new QLabel(tr("Game name"), this);
	editName = new QLineEdit(QString::fromStdString(config.get("player_name")) + "'s game", this);
	
	// FIXME:
	QLabel *labelGametype = new QLabel(tr("Game type"), this);
	QComboBox *comboGametype = new QComboBox(this);
	comboGametype->addItem("THNL");
	comboGametype->setEnabled(false);
	
	// FIXME:
	QLabel *labelGamemode = new QLabel(tr("Game mode"), this);
	QComboBox *comboGamemode = new QComboBox(this);
	comboGamemode->addItem("Sit'n'Go");
	comboGamemode->setEnabled(false);
	
	QGridLayout *layoutGeneral = new QGridLayout;
	layoutGeneral->addWidget(labelName, 0, 0);
	layoutGeneral->addWidget(editName, 0, 1);
	layoutGeneral->addWidget(labelGametype, 1, 0);
	layoutGeneral->addWidget(comboGametype, 1, 1);
	layoutGeneral->addWidget(labelGamemode, 2, 0);
	layoutGeneral->addWidget(comboGamemode, 2, 1);
	groupGeneral->setLayout(layoutGeneral);
	
	QGroupBox *groupPlayers = new QGroupBox(tr("Players"), this);
	
	QLabel *labelPlayers = new QLabel(tr("Max. players"), this);
	spinPlayers = new QSpinBox(this);
	spinPlayers->setMinimum(2);
	spinPlayers->setMaximum(10);
	spinPlayers->setSingleStep(1);
	spinPlayers->setValue(5);
	
	QLabel *labelTimeout = new QLabel(tr("Timeout"), this);
	spinTimeout = new QSpinBox(this);
	spinTimeout->setMinimum(5);
	spinTimeout->setMaximum(3*60);
	spinTimeout->setSingleStep(10);
	spinTimeout->setValue(30);
	
	QLabel *labelStake = new QLabel(tr("Initial stake"), this);
	spinStake = new QSpinBox(this);
	spinStake->setMinimum(500);
	spinStake->setMaximum(1000000);
	spinStake->setSingleStep(100);
	spinStake->setValue(1500);
	
	QGridLayout *layoutPlayers = new QGridLayout;
	layoutPlayers->addWidget(labelPlayers, 0, 0);
	layoutPlayers->addWidget(spinPlayers, 0, 1);
	layoutPlayers->addWidget(labelTimeout, 1, 0);
	layoutPlayers->addWidget(spinTimeout, 1, 1);
	layoutPlayers->addWidget(labelStake, 2, 0);
	layoutPlayers->addWidget(spinStake, 2, 1);
	groupPlayers->setLayout(layoutPlayers);
	
	
	QGroupBox *groupBlinds = new QGroupBox(tr("Blinds"), this);
	
	QLabel *labelBlindsStart = new QLabel(tr("Starting blinds"), this);
	spinBlindsStart = new QSpinBox(this);
	spinBlindsStart->setMinimum(5);
	spinBlindsStart->setMaximum(200);
	spinBlindsStart->setSingleStep(10);
	spinBlindsStart->setValue(20);
	
	QLabel *labelBlindsFactor = new QLabel(tr("Raise factor"), this);
	spinBlindsFactor = new QDoubleSpinBox(this);
	spinBlindsFactor->setDecimals(2);
	spinBlindsFactor->setMinimum(1.3);
	spinBlindsFactor->setMaximum(3.5);
	spinBlindsFactor->setSingleStep(0.1);
	spinBlindsFactor->setValue(2.0);
	
	QLabel *labelBlindsTime = new QLabel(tr("Raise time"), this);
	spinBlindsTime = new QSpinBox(this);
	spinBlindsTime->setMinimum(60);
	spinBlindsTime->setMaximum(10*60);
	spinBlindsTime->setSingleStep(60);
	spinBlindsTime->setValue(5*60);
	
	QGridLayout *layoutBlinds = new QGridLayout;
	layoutBlinds->addWidget(labelBlindsStart, 0, 0);
	layoutBlinds->addWidget(spinBlindsStart, 0, 1);
	layoutBlinds->addWidget(labelBlindsFactor, 1, 0);
	layoutBlinds->addWidget(spinBlindsFactor, 1, 1);
	layoutBlinds->addWidget(labelBlindsTime, 2, 0);
	layoutBlinds->addWidget(spinBlindsTime, 2, 1);
	groupBlinds->setLayout(layoutBlinds);
	
	
	QGroupBox *groupPrivate = new QGroupBox(tr("Private game"), this);
	
	QLabel *labelPrivate = new QLabel(tr("Password protected"), this);
	QCheckBox *checkPrivate = new QCheckBox(this);
	connect(checkPrivate, SIGNAL(stateChanged(int)), this, SLOT(slotCheckStatePrivate(int)));
	
	QLabel *labelPassword = new QLabel(tr("Password"), this);
	editPassword = new QLineEdit(this);
	editPassword->setEnabled(false);
	editPassword->setEchoMode(config.getBool("ui_echo_password") ? QLineEdit::Normal : QLineEdit::Password);
	
	
	QGridLayout *layoutPrivate = new QGridLayout;
	layoutPrivate->addWidget(labelPrivate, 0, 0);
	layoutPrivate->addWidget(checkPrivate, 0, 1);
	layoutPrivate->addWidget(labelPassword, 1, 0);
	layoutPrivate->addWidget(editPassword, 1, 1);
	groupPrivate->setLayout(layoutPrivate);
	
	
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(groupGeneral, 0, 0);
	mainLayout->addWidget(groupPlayers, 0, 1);
	mainLayout->addWidget(groupBlinds, 1, 1);
	mainLayout->addWidget(groupPrivate, 1, 0);
	mainLayout->addWidget(buttonBox, 2, 0, 1, 2);
	setLayout(mainLayout);
}

void CreateGameDialog::actionOk()
{
	bool bError = false;
	
	if (!bError)
	{
		// TODO: validate
		
		accept();
	}
}

void CreateGameDialog::slotCheckStatePrivate(int value)
{
	editPassword->setEnabled(value);
}

QString CreateGameDialog::getName() const
{
	return editName->text();
}

QString CreateGameDialog::getPassword() const
{
	return editPassword->text();
}

chips_type CreateGameDialog::getStake() const
{
	return spinStake->value();
}

unsigned int CreateGameDialog::getPlayers() const
{ 
	return spinPlayers->value();
}

unsigned int CreateGameDialog::getTimeout() const
{ 
	return spinTimeout->value();
}

chips_type CreateGameDialog::getBlindsStart() const
{
	return spinBlindsStart->value();
}

double CreateGameDialog::getBlindsFactor() const
{
	return spinBlindsFactor->value();
}

unsigned int CreateGameDialog::getBlindsTime() const
{ 
	return spinBlindsTime->value();
}
