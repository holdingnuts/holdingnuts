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
#include <QSpinBox>
#include <QDoubleSpinBox>

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
	
	QLabel *labelStake = new QLabel(tr("Initial stake"), this);
	spinStake = new QDoubleSpinBox(this);
	spinStake->setDecimals(2);
	spinStake->setMinimum(500.0);
	spinStake->setMaximum(1000000.0);
	spinStake->setSingleStep(100.0);
	spinStake->setValue(5000.0);
	
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
	
	QLabel *labelBlindsStart = new QLabel(tr("Starting blinds"), this);
	spinBlindsStart = new QDoubleSpinBox(this);
	spinBlindsStart->setDecimals(2);
	spinBlindsStart->setMinimum(5.0);
	spinBlindsStart->setMaximum(200.0);
	spinBlindsStart->setSingleStep(10.0);
	spinBlindsStart->setValue(20.0);
	
	QLabel *labelBlindsFactor = new QLabel(tr("Blinds factor"), this);
	spinBlindsFactor = new QDoubleSpinBox(this);
	spinBlindsFactor->setDecimals(2);
	spinBlindsFactor->setMinimum(1.0);
	spinBlindsFactor->setMaximum(3.5);
	spinBlindsFactor->setSingleStep(0.1);
	spinBlindsFactor->setValue(2.0);
	
	QLabel *labelBlindsTime = new QLabel(tr("Blinds time"), this);
	spinBlindsTime = new QSpinBox(this);
	spinBlindsTime->setMinimum(60);
	spinBlindsTime->setMaximum(10*60);
	spinBlindsTime->setSingleStep(60);
	spinBlindsTime->setValue(3*60);
	
	QGridLayout *layoutGeneral = new QGridLayout;
	layoutGeneral->addWidget(labelName, 0, 0);
	layoutGeneral->addWidget(editName, 0, 1);
	layoutGeneral->addWidget(labelStake, 1, 0);
	layoutGeneral->addWidget(spinStake, 1, 1);
	layoutGeneral->addWidget(labelPlayers, 2, 0);
	layoutGeneral->addWidget(spinPlayers, 2, 1);
	layoutGeneral->addWidget(labelTimeout, 3, 0);
	layoutGeneral->addWidget(spinTimeout, 3, 1);
	layoutGeneral->addWidget(labelBlindsStart, 4, 0);
	layoutGeneral->addWidget(spinBlindsStart, 4, 1);
	layoutGeneral->addWidget(labelBlindsFactor, 5, 0);
	layoutGeneral->addWidget(spinBlindsFactor, 5, 1);
	layoutGeneral->addWidget(labelBlindsTime, 6, 0);
	layoutGeneral->addWidget(spinBlindsTime, 6, 1);
	groupGeneral->setLayout(layoutGeneral);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(groupGeneral);
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
