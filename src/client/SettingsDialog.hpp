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


#ifndef _SETTINGS_DIALOG_H
#define _SETTINGS_DIALOG_H

#include <QDialog>

#include "ConfigParser.hpp"

class QTabWidget;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QLabel;

class SettingsDialog : public QDialog
{
Q_OBJECT

public:
	SettingsDialog(ConfigParser &cp, QWidget *parent = 0);

private:
	ConfigParser *cfg;
	
	// tabGeneral
	QLabel *labelUUIDdisplay;
	QComboBox *comboLocale;
	QCheckBox *checkSound, *checkSoundFocus;
	
	// tabAppearance
	QCheckBox *checkHandStrength;
	QCheckBox *checkCentralView;
	QCheckBox *checkBringToTop;
	QComboBox *comboCarddeck;
	
	// tabPlayerinfo
	QLineEdit *editPlayerName;
	QLineEdit *editPlayerLocation;
	
	// tabLogChat
	QCheckBox *checkLog, *checkLogChat;
	QCheckBox *checkVerboseFoyerTime;
	QCheckBox *checkVerboseFoyerJoinLeft;
	QCheckBox *checkVerboseFoyerGameState;
	QCheckBox *checkVerboseFoyerPlayerChat;
	QCheckBox *checkVerboseTablePlayerActions;
	QCheckBox *checkVerboseTableCards;
	QCheckBox *checkVerboseTablePlayerChat;
	
private slots:
	void actionOk();
	void actionGenUUID();
	void actionClearUUID();
	void actionCheckStateSound(int new_state);
	void actionCheckStateLog(int new_state);
};

#endif /* _SETTINGS_DIALOG_H */
