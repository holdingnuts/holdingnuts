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
	
	QLabel *labelPlayers = new QLabel(tr("Max. Players"), this);
	spinPlayers = new QSpinBox(this);
	spinPlayers->setMinimum(2);
	spinPlayers->setMaximum(10);
	spinPlayers->setSingleStep(1);
	spinPlayers->setValue(10);
	
	QLabel *labelTimeout = new QLabel(tr("Timeout"), this);
	spinTimeout = new QSpinBox(this);
	spinTimeout->setMinimum(5);
	spinTimeout->setMaximum(3*60);
	spinTimeout->setSingleStep(10);
	spinTimeout->setValue(30);
	
	QLabel *labelStake = new QLabel(tr("Initial stake"), this);
	spinStake = new QDoubleSpinBox(this);
	spinStake->setDecimals(2);
	spinStake->setMinimum(500.0);
	spinStake->setMaximum(1000000.0);
	spinStake->setSingleStep(100.0);
	spinStake->setValue(5000.0);
	
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
	spinBlindsStart = new QDoubleSpinBox(this);
	spinBlindsStart->setDecimals(2);
	spinBlindsStart->setMinimum(5.0);
	spinBlindsStart->setMaximum(200.0);
	spinBlindsStart->setSingleStep(10.0);
	spinBlindsStart->setValue(20.0);
	
	QLabel *labelBlindsFactor = new QLabel(tr("Raise factor"), this);
	spinBlindsFactor = new QDoubleSpinBox(this);
	spinBlindsFactor->setDecimals(2);
	spinBlindsFactor->setMinimum(1.0);
	spinBlindsFactor->setMaximum(3.5);
	spinBlindsFactor->setSingleStep(0.1);
	spinBlindsFactor->setValue(2.0);
	
	QLabel *labelBlindsTime = new QLabel(tr("Raise time"), this);
	spinBlindsTime = new QSpinBox(this);
	spinBlindsTime->setMinimum(60);
	spinBlindsTime->setMaximum(10*60);
	spinBlindsTime->setSingleStep(60);
	spinBlindsTime->setValue(3*60);
	
	QGridLayout *layoutBlinds = new QGridLayout;
	layoutBlinds->addWidget(labelBlindsStart, 0, 0);
	layoutBlinds->addWidget(spinBlindsStart, 0, 1);
	layoutBlinds->addWidget(labelBlindsFactor, 1, 0);
	layoutBlinds->addWidget(spinBlindsFactor, 1, 1);
	layoutBlinds->addWidget(labelBlindsTime, 2, 0);
	layoutBlinds->addWidget(spinBlindsTime, 2, 1);
	groupBlinds->setLayout(layoutBlinds);
	
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(groupGeneral);
	mainLayout->addWidget(groupPlayers);
	mainLayout->addWidget(groupBlinds);
	mainLayout->addWidget(buttonBox);
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

QString CreateGameDialog::getName()
{
	return editName->text();
}

float CreateGameDialog::getStake()
{
	return spinStake->value();
}

unsigned int CreateGameDialog::getPlayers()
{ 
	return spinPlayers->value();
}

unsigned int CreateGameDialog::getTimeout()
{ 
	return spinTimeout->value();
}

float CreateGameDialog::getBlindsStart()
{
	return spinBlindsStart->value();
}

float CreateGameDialog::getBlindsFactor()
{
	return spinBlindsFactor->value();
}

unsigned int CreateGameDialog::getBlindsTime()
{ 
	return spinBlindsTime->value();
}
