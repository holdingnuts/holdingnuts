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

#include "SettingsDialog.hpp"


SettingsDialog::SettingsDialog(const char *filename, ConfigParser &cp, QWidget *parent) : QDialog(parent)
{
	cfg = &cp;
	configfile = filename;
	
	setWindowTitle(tr("Settings"));
	
	tabWidget = new QTabWidget;
	tabGeneral = new QWidget;
	tabWidget->addTab(tabGeneral, tr("General"));
	tabWidget->addTab(new QWidget(), tr("Appearance"));
	
	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
					| QDialogButtonBox::Cancel);
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(actionOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
	
	
	// tabGeneral
	QLabel *labelPlayerName = new QLabel(tr("Player name:"), tabGeneral);
	editPlayerName = new QLineEdit(QString::fromStdString(cfg->get("player_name")), tabGeneral);
	
	QLabel *labelUUID = new QLabel(tr("UUID:"), tabGeneral);
	editUUID = new QLineEdit(QString::fromStdString(cfg->get("uuid")), tabGeneral);
	
	QLabel *labelLog = new QLabel(tr("Log to file:"), tabGeneral);
	checkLog = new QCheckBox("", tabGeneral);
	checkLog->setCheckState(cfg->getBool("log") ? Qt::Checked : Qt::Unchecked);
	
	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->addWidget(labelPlayerName, 0, 0);
	gridLayout->addWidget(editPlayerName, 0, 1);
	gridLayout->addWidget(labelUUID, 1, 0);
	gridLayout->addWidget(editUUID, 1, 1);
	gridLayout->addWidget(labelLog, 2, 0);
	gridLayout->addWidget(checkLog, 2, 1);
	tabGeneral->setLayout(gridLayout);
}

void SettingsDialog::actionOk()
{
	bool bError = false;
	
	if (!bError)
	{
		// tabGeneral
		cfg->set("player_name", editPlayerName->text().toStdString());
		cfg->set("uuid", editUUID->text().toStdString());
		cfg->set("log", (checkLog->checkState() == Qt::Checked) ? true : false);
		
		cfg->save(configfile);
		
		accept();
	}
}
