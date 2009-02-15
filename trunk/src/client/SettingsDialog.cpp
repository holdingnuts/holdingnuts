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


SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
	tabWidget = new QTabWidget;
	tabWidget->addTab(new GeneralTab(), tr("General"));
	
	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
					| QDialogButtonBox::Cancel);
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
	
	setWindowTitle(tr("Settings"));
}

GeneralTab::GeneralTab(QWidget *parent) : QWidget(parent)
{
	QLabel *labelTest = new QLabel(tr("Test"));
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(labelTest);
	setLayout(mainLayout);
}
