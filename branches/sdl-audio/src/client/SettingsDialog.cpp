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
#include "SettingsDialog.hpp"

SettingsDialog::SettingsDialog(const char *filename, ConfigParser &cp, QWidget *parent) : QDialog(parent)
{
	cfg = &cp;
	configfile = filename;
	
	setWindowTitle(tr("Settings"));
	setWindowIcon(QIcon(":/res/pclient.ico"));
	setMinimumWidth(300);
	
	tabWidget = new QTabWidget;
	tabGeneral = new QWidget;
	tabAppearance = new QWidget;
	tabWidget->addTab(tabGeneral, tr("General"));
	tabWidget->addTab(tabAppearance, tr("Appearance"));
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
					  Qt::Horizontal, this);
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(actionOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
	
	
	// tabGeneral
	QLabel *labelPlayerName = new QLabel(tr("Player name"), tabGeneral);
	editPlayerName = new QLineEdit(QString::fromStdString(cfg->get("player_name")), tabGeneral);
	
	QLabel *labelUUID = new QLabel(tr("UUID"), tabGeneral);
	editUUID = new QLineEdit(QString::fromStdString(cfg->get("uuid")), tabGeneral);
	editUUID->setReadOnly(true);
	
	QPushButton *btnUUIDGen = new QPushButton("*", tabGeneral);
	connect(btnUUIDGen, SIGNAL(clicked()), this, SLOT(actionGenUUID()));
	
	QHBoxLayout *layoutUUID = new QHBoxLayout;
	layoutUUID->addWidget(editUUID);
	layoutUUID->addWidget(btnUUIDGen);
	
	QLabel *labelLog = new QLabel(tr("Log to file"), tabGeneral);
	checkLog = new QCheckBox("", tabGeneral);
	checkLog->setCheckState(cfg->getBool("log") ? Qt::Checked : Qt::Unchecked);
	
	QLabel *labelLocale = new QLabel(tr("Locale"), tabGeneral);
	comboLocale = new QComboBox(tabGeneral);
	
	// locales (Note: names are not translated)
	struct {
		QString lId;
		QString lName;
	} locales[] = {
		{ "",	tr("Auto-Detect") },
		{ "en",	"English" },
		{ "de",	"Deutsch" }
	};
	const unsigned int locales_count = sizeof(locales) / sizeof(locales[0]);
	
	for (unsigned int i=0; i < locales_count; i++)
	{
		comboLocale->addItem(locales[i].lName, locales[i].lId);
		if (locales[i].lId.toStdString() == cfg->get("locale"))
			comboLocale->setCurrentIndex(i);
	}
	
	QGridLayout *gridGeneral = new QGridLayout;
	gridGeneral->addWidget(labelPlayerName, 0, 0);
	gridGeneral->addWidget(editPlayerName, 0, 1);
	gridGeneral->addWidget(labelUUID, 1, 0);
	gridGeneral->addLayout(layoutUUID, 1, 1);
	gridGeneral->addWidget(labelLog, 2, 0);
	gridGeneral->addWidget(checkLog, 2, 1);
	gridGeneral->addWidget(labelLocale, 3, 0);
	gridGeneral->addWidget(comboLocale, 3, 1);
	tabGeneral->setLayout(gridGeneral);
	
	
	// tabAppearance
	QLabel *labelHandStrength = new QLabel(tr("Show strength of hand"), tabAppearance);
	checkHandStrength = new QCheckBox("", tabAppearance);
	checkHandStrength->setCheckState(cfg->getBool("ui_show_handstrength") ? Qt::Checked : Qt::Unchecked);
	
	QGridLayout *gridAppearance = new QGridLayout;
	gridAppearance->addWidget(labelHandStrength, 0, 0);
	gridAppearance->addWidget(checkHandStrength, 0, 1);
	tabAppearance->setLayout(gridAppearance);
}

void SettingsDialog::actionGenUUID()
{
	// generate an UUID
	QUuid uuid = QUuid::createUuid();
	QString suuid = uuid.toString();
	suuid.chop(1);
	
	editUUID->setText(suuid.remove(0, 1));
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
		cfg->set("locale", comboLocale->itemData(comboLocale->currentIndex()).toString().toStdString());
		
		// tabAppearance
		cfg->set("ui_show_handstrength", (checkHandStrength->checkState() == Qt::Checked) ? true : false);
		
		cfg->save(configfile);
		
		accept();
	}
}
