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


#include <QtGui>

#include "Debug.h"
#include "SettingsDialog.hpp"

SettingsDialog::SettingsDialog(ConfigParser &cp, QWidget *parent)
:	QDialog(parent),
	cfg(&cp)
{
	setWindowTitle(tr("Settings"));
	setWindowIcon(QIcon(":/res/hn_logo.png"));
	setMinimumWidth(300);
	
	
	QTabWidget *tabWidget = new QTabWidget(this);
	QWidget *tabGeneral = new QWidget;
	QWidget *tabPlayerinfo = new QWidget;
	QWidget *tabAppearance = new QWidget;
	tabWidget->addTab(tabGeneral, tr("General"));
	tabWidget->addTab(tabPlayerinfo, tr("Player info"));
	tabWidget->addTab(tabAppearance, tr("Appearance"));
	
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
					  Qt::Horizontal, this);
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(actionOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
	
	
	// --- tabGeneral ---
	
	// logging options
	checkLog = new QCheckBox(tr("enabled"), tabGeneral);
	checkLog->setCheckState(cfg->getBool("log") ? Qt::Checked : Qt::Unchecked);
	checkLogChat = new QCheckBox(tr("log chat"), tabGeneral);
	checkLogChat->setCheckState(cfg->getBool("log_chat") ? Qt::Checked : Qt::Unchecked);
	
	actionCheckStateLog(checkLog->checkState());
	connect(checkLog, SIGNAL(stateChanged(int)), this, SLOT(actionCheckStateLog(int)));
	
	QHBoxLayout *layoutLog = new QHBoxLayout;
	layoutLog->addWidget(checkLog);
	layoutLog->addWidget(checkLogChat);
	
	// time in chatbox
	checkTimeInFoyerChat = new QCheckBox(tr("enabled"), tabGeneral);
	checkTimeInFoyerChat->setCheckState(cfg->getBool("time_in_foyerchat") ? Qt::Checked : Qt::Unchecked);
	
	comboLocale = new QComboBox(tabGeneral);
	
	// locales (Note: names are not translated)
	struct {
		QString lId;
		QString lName;
	} locales[] = {
		{ "",	tr("Auto-Detect") },
		{ "de",	"German" },
		{ "en",	"English" },
		{ "ja", "Japanese" },
		{ "nl", "Dutch" },
		{ "ro", "Romanian" },
	};
	const unsigned int locales_count = sizeof(locales) / sizeof(locales[0]);
	
	for (unsigned int i=0; i < locales_count; i++)
	{
		comboLocale->addItem(locales[i].lName, locales[i].lId);
		if (locales[i].lId.toStdString() == cfg->get("locale"))
			comboLocale->setCurrentIndex(i);
	}
	
	// sound options
	checkSound = new QCheckBox(tr("enabled"), tabGeneral);
	checkSound->setCheckState(cfg->getBool("sound") ? Qt::Checked : Qt::Unchecked);
	checkSoundFocus = new QCheckBox(tr("only on focus"), tabGeneral);
	checkSoundFocus->setCheckState(cfg->getBool("sound_focus") ? Qt::Checked : Qt::Unchecked);
	
	actionCheckStateSound(checkSound->checkState());
	connect(checkSound, SIGNAL(stateChanged(int)), this, SLOT(actionCheckStateSound(int)));
	
	QHBoxLayout *layoutSound = new QHBoxLayout;
	layoutSound->addWidget(checkSound);
	layoutSound->addWidget(checkSoundFocus);
	
	// UUID
	labelUUIDdisplay = new QLabel(QString::fromStdString(cfg->get("uuid")), tabGeneral);
	labelUUIDdisplay->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	
	QPushButton *btnUUIDGen = new QPushButton(tr("generate"), tabGeneral);
	connect(btnUUIDGen, SIGNAL(clicked()), this, SLOT(actionGenUUID()));
	
	QPushButton *btnUUIDClear = new QPushButton(tr("clear"), tabGeneral);
	connect(btnUUIDClear, SIGNAL(clicked()), this, SLOT(actionClearUUID()));
	
	
	QHBoxLayout *layoutUUIDButtons = new QHBoxLayout;
	layoutUUIDButtons->addWidget(btnUUIDGen);
	layoutUUIDButtons->addWidget(btnUUIDClear);
	
	QVBoxLayout *layoutUUID = new QVBoxLayout;
	layoutUUID->addWidget(labelUUIDdisplay);
	layoutUUID->addLayout(layoutUUIDButtons);
	
	QFormLayout *formGeneral = new QFormLayout;
	formGeneral->addRow(tr("Log to file"), layoutLog);
	formGeneral->addRow(tr("Show Time in Foyerchat"), checkTimeInFoyerChat);
	formGeneral->addRow(tr("Locale"), comboLocale);
	formGeneral->addRow(tr("Sounds"), layoutSound);
	formGeneral->addRow(tr("UUID"), layoutUUID);
	tabGeneral->setLayout(formGeneral);

	
	// --- tabPlayerinfo ---
	editPlayerName = new QLineEdit(QString::fromStdString(cfg->get("player_name")), tabPlayerinfo);
	editPlayerLocation = new QLineEdit(QString::fromStdString(cfg->get("info_location")), tabPlayerinfo);

	QFormLayout *formPlayerinfo = new QFormLayout;
	formPlayerinfo->addRow(tr("Player name"), editPlayerName);
	formPlayerinfo->addRow(tr("Location"), editPlayerLocation);
	tabPlayerinfo->setLayout(formPlayerinfo);
	
	
	// --- tabAppearance ---
	checkHandStrength = new QCheckBox("", tabAppearance);
	checkHandStrength->setCheckState(cfg->getBool("ui_show_handstrength") ? Qt::Checked : Qt::Unchecked);

	checkCentralView = new QCheckBox("", tabAppearance);
	checkCentralView->setCheckState(cfg->getBool("ui_centralized_view") ? Qt::Checked : Qt::Unchecked);

	checkBringOnTop = new QCheckBox("", tabAppearance);
	checkBringOnTop->setCheckState(cfg->getBool("ui_bring_on_top") ? Qt::Checked : Qt::Unchecked);
	
	comboCarddeck = new QComboBox(tabAppearance);
	
	// card decks  // FIXME: retrieve directory listing
	struct {
		QString lId;
		QString lName;
	} decks[] = {
		{ "default",	tr("Default") },
		{ "classic",	tr("Classic") }
	};
	const unsigned int decks_count = sizeof(decks) / sizeof(decks[0]);
	
	for (unsigned int i=0; i < decks_count; i++)
	{
		comboCarddeck->addItem(decks[i].lName, decks[i].lId);
		if (decks[i].lId.toStdString() == cfg->get("ui_card_deck"))
			comboCarddeck->setCurrentIndex(i);
	}
	
	QFormLayout *formAppearance = new QFormLayout;
	formAppearance->addRow(tr("Show strength of hand"), checkHandStrength);
	formAppearance->addRow(tr("Centralized table view"), checkCentralView);
	formAppearance->addRow(tr("Bring Window on Top"), checkBringOnTop);
	formAppearance->addRow(tr("Card deck"), comboCarddeck);
	tabAppearance->setLayout(formAppearance);
}

void SettingsDialog::actionGenUUID()
{
	// generate an UUID
	QString suuid = QUuid::createUuid().toString();
	suuid = suuid.mid(1, suuid.length() - 2);
	
	labelUUIDdisplay->setText(suuid);
}

void SettingsDialog::actionClearUUID()
{
	// unset UUID
	labelUUIDdisplay->clear();
}

void SettingsDialog::actionOk()
{
	bool bError = false;
	
	if (!bError)
	{
		// tabGeneral
		cfg->set("uuid", labelUUIDdisplay->text().toStdString());
		cfg->set("log", (checkLog->checkState() == Qt::Checked) ? true : false);
		cfg->set("log_chat", (checkLogChat->checkState() == Qt::Checked) ? true : false);
		cfg->set("locale", comboLocale->itemData(comboLocale->currentIndex()).toString().toStdString());
		cfg->set("sound", (checkSound->checkState() == Qt::Checked) ? true : false);
		cfg->set("sound_focus", (checkSoundFocus->checkState() == Qt::Checked) ? true : false);
		cfg->set("time_in_foyerchat", (checkSoundFocus->checkState() == Qt::Checked) ? true : false);
		
		// tabPlayerinfo
		cfg->set("player_name", editPlayerName->text().toStdString());
		cfg->set("info_location", editPlayerLocation->text().toStdString());
		
		// tabAppearance
		cfg->set("ui_show_handstrength", (checkHandStrength->checkState() == Qt::Checked) ? true : false);
		cfg->set("ui_centralized_view", (checkCentralView->checkState() == Qt::Checked) ? true : false);
		cfg->set("ui_bring_on_top", (checkBringOnTop->checkState() == Qt::Checked) ? true : false);
		cfg->set("ui_card_deck", comboCarddeck->itemData(comboCarddeck->currentIndex()).toString().toStdString());
		
		accept();
	}
}

void SettingsDialog::actionCheckStateSound(int new_state)
{
	checkSoundFocus->setEnabled(
		((Qt::CheckState)new_state == Qt::Checked) ? true : false);
}

void SettingsDialog::actionCheckStateLog(int new_state)
{
	checkLogChat->setEnabled(
		((Qt::CheckState)new_state == Qt::Checked) ? true : false);
}
