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
	QWidget *tabLogChat = new QWidget;
	
	tabWidget->addTab(tabGeneral, tr("General"));
	tabWidget->addTab(tabPlayerinfo, tr("Player info"));
	tabWidget->addTab(tabAppearance, tr("Appearance"));
	tabWidget->addTab(tabLogChat, tr("Log and chat"));
	
	
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
					  Qt::Horizontal, this);
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(actionOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
	
	
	// --- tabGeneral ---
	comboLocale = new QComboBox(tabGeneral);
	
	// locales (Note: names are not being translated)
	struct {
		QString lId;
		QString lName;
	} locales[] = {
		{ "",	tr("Auto-Detect") },
		{ "de",	"German" },
		{ "en",	"English" },
		{ "it", "Italian" },
		{ "ja", "Japanese" },
		{ "nl", "Dutch" },
		{ "ro", "Romanian" },
		{ "ru", "Russian" },
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
	formGeneral->addRow(tr("Locale"), comboLocale);
	formGeneral->addRow(tr("Sounds"), layoutSound);
	formGeneral->addRow(tr("UUID"), layoutUUID);
	tabGeneral->setLayout(formGeneral);

	
	// --- tabPlayerinfo ---
	editPlayerName = new QLineEdit(QString::fromStdString(cfg->get("player_name")), tabPlayerinfo);
	editPlayerName->setMaxLength(20);

	editPlayerLocation = new QLineEdit(QString::fromStdString(cfg->get("info_location")), tabPlayerinfo);
	editPlayerLocation->setMaxLength(30);

	QFormLayout *formPlayerinfo = new QFormLayout;
	formPlayerinfo->addRow(tr("Player name"), editPlayerName);
	formPlayerinfo->addRow(tr("Location"), editPlayerLocation);
	tabPlayerinfo->setLayout(formPlayerinfo);
	
	
	// --- tabAppearance ---
	checkHandStrength = new QCheckBox("", tabAppearance);
	checkHandStrength->setCheckState(cfg->getBool("ui_show_handstrength") ? Qt::Checked : Qt::Unchecked);

	checkCentralView = new QCheckBox("", tabAppearance);
	checkCentralView->setCheckState(cfg->getBool("ui_centralized_view") ? Qt::Checked : Qt::Unchecked);

	checkBringToTop = new QCheckBox("", tabAppearance);
	checkBringToTop->setCheckState(cfg->getBool("ui_bring_to_top") ? Qt::Checked : Qt::Unchecked);
	
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
	formAppearance->addRow(tr("Bring window to top"), checkBringToTop);
	formAppearance->addRow(tr("Card deck"), comboCarddeck);
	tabAppearance->setLayout(formAppearance);
	
	
	// --- tabLogChat ---
	
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
	
	// verbosity level foyer
	checkVerboseFoyerTime = new QCheckBox(tr("Display time in foyer chat"), tabGeneral);	// 0x1
	checkVerboseFoyerTime->setCheckState((cfg->getInt("chat_verbosity_foyer") & 0x1) ? Qt::Checked : Qt::Unchecked);
	
	checkVerboseFoyerJoinLeft = new QCheckBox(tr("Display join/left messages"), tabGeneral);	// 0x2
	checkVerboseFoyerJoinLeft->setCheckState((cfg->getInt("chat_verbosity_foyer") & 0x2) ? Qt::Checked : Qt::Unchecked);

	checkVerboseFoyerGameState = new QCheckBox(tr("Display game states"), tabGeneral);			// 0x4
	checkVerboseFoyerGameState->setCheckState((cfg->getInt("chat_verbosity_foyer") & 0x4) ? Qt::Checked : Qt::Unchecked);

	checkVerboseFoyerPlayerChat = new QCheckBox(tr("Display player chat"), tabGeneral);			// 0x8
	checkVerboseFoyerPlayerChat->setCheckState((cfg->getInt("chat_verbosity_foyer") & 0x8) ? Qt::Checked : Qt::Unchecked);

	// verbosity level table
	checkVerboseTablePlayerActions = new QCheckBox(tr("Display player actions"), tabGeneral);	// 0x1
	checkVerboseTablePlayerActions->setCheckState((cfg->getInt("chat_verbosity_table") & 0x1) ? Qt::Checked : Qt::Unchecked);

	checkVerboseTableCards = new QCheckBox(tr("Display hole/community cards"), tabGeneral);	// 0x2
	checkVerboseTableCards->setCheckState((cfg->getInt("chat_verbosity_table") & 0x2) ? Qt::Checked : Qt::Unchecked);

	checkVerboseTablePlayerChat = new QCheckBox(tr("Display player chat"), tabGeneral);	// 0x4
	checkVerboseTablePlayerChat->setCheckState((cfg->getInt("chat_verbosity_table") & 0x4) ? Qt::Checked : Qt::Unchecked);

	QFormLayout *formLogChat = new QFormLayout;
	formLogChat->addRow(tr("Log to file"), layoutLog);
	formLogChat->addRow(tr("Foyer chat verbosity"), checkVerboseFoyerTime);
	formLogChat->addRow(" ", checkVerboseFoyerJoinLeft);
	formLogChat->addRow(" ", checkVerboseFoyerGameState);
	formLogChat->addRow(" ", checkVerboseFoyerPlayerChat);
	formLogChat->addRow(tr("Table chat verbosity"), checkVerboseTablePlayerActions);
	formLogChat->addRow(" ", checkVerboseTableCards);
	formLogChat->addRow(" ", checkVerboseTablePlayerChat);
	tabLogChat->setLayout(formLogChat);
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
	
	// FIXME: validate settings
	if (!bError)
	{
		// tabGeneral
		cfg->set("uuid", labelUUIDdisplay->text().toStdString());
		cfg->set("locale", comboLocale->itemData(comboLocale->currentIndex()).toString().toStdString());
		cfg->set("sound", (checkSound->checkState() == Qt::Checked) ? true : false);
		cfg->set("sound_focus", (checkSoundFocus->checkState() == Qt::Checked) ? true : false);
		
		
		// tabPlayerinfo
		cfg->set("player_name", editPlayerName->text().toStdString());
		cfg->set("info_location", editPlayerLocation->text().toStdString());
		
		
		// tabAppearance
		cfg->set("ui_show_handstrength", (checkHandStrength->checkState() == Qt::Checked) ? true : false);
		cfg->set("ui_centralized_view", (checkCentralView->checkState() == Qt::Checked) ? true : false);
		cfg->set("ui_bring_to_top", (checkBringToTop->checkState() == Qt::Checked) ? true : false);
		cfg->set("ui_card_deck", comboCarddeck->itemData(comboCarddeck->currentIndex()).toString().toStdString());
		
		
		// tabLogChat
		cfg->set("log", (checkLog->checkState() == Qt::Checked) ? true : false);
		cfg->set("log_chat", (checkLogChat->checkState() == Qt::Checked) ? true : false);
		
		// verbosity level foyer
		int chat_verbosity_foyer = 0;
		
		if (checkVerboseFoyerTime->checkState() == Qt::Checked)
			chat_verbosity_foyer |= 0x1;
		if (checkVerboseFoyerJoinLeft->checkState() == Qt::Checked)
			chat_verbosity_foyer |= 0x2;
		if (checkVerboseFoyerGameState->checkState() == Qt::Checked)
			chat_verbosity_foyer |= 0x4;			
		if (checkVerboseFoyerPlayerChat->checkState() == Qt::Checked)
			chat_verbosity_foyer |= 0x8;
			
		cfg->set("chat_verbosity_foyer", chat_verbosity_foyer);
		
		// verbosity level table
		int chat_verbosity_table = 0;
		
		if (checkVerboseTablePlayerActions->checkState() == Qt::Checked)
			chat_verbosity_table |= 0x1;
		if (checkVerboseTableCards->checkState() == Qt::Checked)
			chat_verbosity_table |= 0x2;
		if (checkVerboseTablePlayerChat->checkState() == Qt::Checked)
			chat_verbosity_table |= 0x4;

		cfg->set("chat_verbosity_table", chat_verbosity_table);
		
		
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
