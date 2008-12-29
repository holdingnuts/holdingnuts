/*
 * Copyright 2008, Dominik Geyer
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
 */


#include <cstdio>
#include "WTable.hpp"

#include "Config.h"
#include "Debug.h"

#include "pclient.hpp"

WTable::WTable(QWidget *parent) : QWidget(parent)
{
/*
	0	1	2	3	4	5	6	7	8	9	10
0			_						_		
1				S		D		S			
2		S								S	
3	S										S
4		S								S	
5					S		S				
*/
	QGridLayout *grid_seats = new QGridLayout();
	
	QLabel *lDealer = new QLabel("Dealer", this);
	grid_seats->addWidget(lDealer, 1, 5);
	
	QLabel *lSpacer1 = new QLabel(" ", this);
	QLabel *lSpacer2 = new QLabel(" ", this);
	grid_seats->addWidget(lSpacer1, 0, 2);
	grid_seats->addWidget(lSpacer2, 0, 8);
	
	struct seatinfo {
		QString name;
		int col;  // x
		int row;  // y
	} seats[] = {
		{ "Seat 1",  7, 1 },
		{ "Seat 2",  9, 2 },
		{ "Seat 3",  10, 3 },
		{ "Seat 4",  9, 4 },
		{ "Seat 5",  6, 5 },
		{ "Seat 6",  4, 5 },
		{ "Seat 7",  1, 4 },
		{ "Seat 8",  0, 3 },
		{ "Seat 9",  1, 2 },
		{ "Seat 10", 3, 1 }
	};
	
	const int seatcount = sizeof(seats) / sizeof(seats[0]);
	
	for (int i=0; i < seatcount; i++)
	{
		WSeat *seat = new WSeat(i, this);
		grid_seats->addWidget(seat, seats[i].row, seats[i].col);
	}
	
	///////
	
	QHBoxLayout *lActions = new QHBoxLayout();
	
	QButtonGroup *btngrpActions = new QButtonGroup();
	btngrpActions->setExclusive(true);
	
	QPushButton *btnFold = new QPushButton(tr("Fold"), this);
	btnFold->setCheckable(true);
	QPushButton *btnCheckCall = new QPushButton(tr("Check/Call"), this);
	btnCheckCall->setCheckable(true);
	QPushButton *btnBetRaise = new QPushButton(tr("Bet/Raise"), this);
	btnBetRaise->setCheckable(true);
	
	btngrpActions->addButton(btnFold);
	btngrpActions->addButton(btnCheckCall);
	btngrpActions->addButton(btnBetRaise);
	
	QVBoxLayout *lAmount = new QVBoxLayout();
	
	QLineEdit *editAmount = new QLineEdit("0", this);
	QSlider *sliderAmount = new QSlider(Qt::Horizontal, this);
	
	lAmount->addWidget(editAmount);
	lAmount->addWidget(sliderAmount);
	
	lActions->addWidget(btnFold);
	lActions->addWidget(btnCheckCall);
	lActions->addWidget(btnBetRaise);
	lActions->addLayout(lAmount);
	
	///////
	
	QGridLayout *layout = new QGridLayout();
	layout->addLayout(grid_seats, 1, 0);
	layout->setRowStretch(1, 90);
	layout->addLayout(lActions, 2, 0);
	layout->setRowStretch(2, 1);

	setLayout(layout);
}

WSeat::WSeat(unsigned int id, QWidget *parent) : QWidget(parent)
{
	setPalette(Qt::gray);
	setAutoFillBackground(true);
	
	QVBoxLayout *layout = new QVBoxLayout();
	
	QLabel *lblCaption = new QLabel(tr("Seat"), this);
	lblCaption->setAlignment(Qt::AlignCenter);
	QLabel *lblStake = new QLabel(tr("123.45"), this);
	lblStake->setAlignment(Qt::AlignCenter);
	QLabel *lblAction = new QLabel(tr("Action"), this);
	lblAction->setAlignment(Qt::AlignCenter);
	
	layout->addWidget(lblCaption);
	layout->addWidget(lblStake);
	layout->addWidget(lblAction);
	
	setLayout(layout);
}
