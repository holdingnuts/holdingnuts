/*
 * Copyright 2008-2010, Dominik Geyer
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


#ifndef _FIRSTSTART_DIALOG_H
#define _FIRSTSTART_DIALOG_H

#include <QDialog>

#include "ConfigParser.hpp"

class QLineEdit;
class QCheckBox;
class QLabel;

class FirstStartDialog : public QDialog
{
Q_OBJECT

private:
	ConfigParser *cfg;

	QLineEdit *editPlayerName;
	QCheckBox *checkUUID;

public:
	FirstStartDialog(ConfigParser &cp, QWidget *parent = 0);

private slots:
	void actionOk();
};

#endif /* _FIRSTSTART_DIALOG_H */
