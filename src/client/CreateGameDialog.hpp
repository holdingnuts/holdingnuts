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


#ifndef _CREATEGAME_DIALOG_H
#define _CREATEGAME_DIALOG_H

#include <QDialog>

class WMain;
class QTabWidget;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;

class CreateGameDialog : public QDialog
{
Q_OBJECT

friend class WMain;

public:
	CreateGameDialog(QWidget *parent = 0);
	
	QString getName();
	double getStake();
	unsigned int getPlayers();
	unsigned int getTimeout();
	double getBlindsStart();
	double getBlindsFactor();
	unsigned int getBlindsTime();

private:
	QLineEdit	*editName;
	QDoubleSpinBox  *spinStake;
	QSpinBox	*spinPlayers;
	QSpinBox	*spinTimeout;
	QDoubleSpinBox  *spinBlindsStart;
	QDoubleSpinBox  *spinBlindsFactor;
	QSpinBox        *spinBlindsTime;
	
private slots:
	void actionOk();
};

#endif /* _CREATEGAME_DIALOG_H */
