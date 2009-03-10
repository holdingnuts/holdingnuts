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
	
	QGridLayout *layoutGeneral = new QGridLayout;
	layoutGeneral->addWidget(labelName, 0, 0);
	layoutGeneral->addWidget(editName, 0, 1);
	layoutGeneral->addWidget(labelStake, 1, 0);
	layoutGeneral->addWidget(spinStake, 1, 1);
	layoutGeneral->addWidget(labelPlayers, 2, 0);
	layoutGeneral->addWidget(spinPlayers, 2, 1);
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
