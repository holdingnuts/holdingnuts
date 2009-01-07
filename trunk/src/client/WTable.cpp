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

using namespace std;


WTable::WTable(int gid, int tid, QWidget *parent) : QWidget(parent)
{
	this->gid = gid;
	this->tid = tid;
	
	setWindowTitle("HoldingNuts table");
	//setAttribute(Qt::WA_DeleteOnClose); // FIXME:
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
	
	QLabel *lDealer = new QLabel(tr("Dealer"), this);
	lDealer->setAlignment(Qt::AlignCenter);
	grid_seats->addWidget(lDealer, 1, 5);
	
	QLabel *lSpacer1 = new QLabel(" ", this);
	QLabel *lSpacer2 = new QLabel(" ", this);
	grid_seats->addWidget(lSpacer1, 1, 2);
	grid_seats->addWidget(lSpacer2, 1, 8);
	
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
		wseats[i] = new WSeat(i, this);
		grid_seats->addWidget(wseats[i], seats[i].row, seats[i].col);
	}
	
	lblPots = new QLabel("Pots", this);
	lblPots->setAlignment(Qt::AlignCenter);
	grid_seats->addWidget(lblPots, 3, 5);
	
	QHBoxLayout *lCC = new QHBoxLayout();
	for (unsigned int i=0; i < 5; i++)
	{
		cc[i] = new WPicture("gfx/deck/default/back.png", this);
		cc[i]->setFixedSize(45, 60);
		lCC->addWidget(cc[i]);
	}
	
	grid_seats->addLayout(lCC, 2, 4, 1, 3);
	
	///////
	
	QHBoxLayout *lActions = new QHBoxLayout();
	
	//QButtonGroup *btngrpActions = new QButtonGroup();
	//btngrpActions->setExclusive(true);
	
	QPushButton *btnFold = new QPushButton(tr("Fold"), this);
	//btnFold->setCheckable(true);
	connect(btnFold, SIGNAL(clicked()), this, SLOT(actionFold()));
	
	QPushButton *btnCheckCall = new QPushButton(tr("Check/Call"), this);
	//btnCheckCall->setCheckable(true);
	connect(btnCheckCall, SIGNAL(clicked()), this, SLOT(actionCheckCall()));
	
	QPushButton *btnBetRaise = new QPushButton(tr("Bet/Raise"), this);
	connect(btnBetRaise, SIGNAL(clicked()), this, SLOT(actionBetRaise()));
	
	// FIXME:
	QPushButton *btnShow = new QPushButton(tr("Show"), this);
	connect(btnShow, SIGNAL(clicked()), this, SLOT(actionShow()));
	
	//btnBetRaise->setCheckable(true);
	
	//btngrpActions->addButton(btnFold);
	//btngrpActions->addButton(btnCheckCall);
	//btngrpActions->addButton(btnBetRaise);
	
	QVBoxLayout *lAmount = new QVBoxLayout();
	
	editAmount = new QLineEdit("0", this);
	editAmount->setAlignment(Qt::AlignRight);
	
	
	sliderAmount = new QSlider(Qt::Horizontal, this);
	//sliderAmount->setEnabled(false);
	sliderAmount->setTickPosition(QSlider::TicksBothSides);
	sliderAmount->setTickInterval(10);
	sliderAmount->setSingleStep(1);
	sliderAmount->setMinimum(0);
	sliderAmount->setMaximum(100);
	connect(sliderAmount, SIGNAL(valueChanged(int)), this, SLOT(slotBetValue(int)));
	
	
	lAmount->addWidget(editAmount);
	lAmount->addWidget(sliderAmount);
	
	lActions->addWidget(btnFold);
	lActions->addWidget(btnCheckCall);
	lActions->addWidget(btnBetRaise);
	
	lActions->addWidget(btnShow);
	
	lActions->addLayout(lAmount);
	
	QWidget *wActions = new QWidget(this);
	//wActions->setPalette(Qt::gray);
	//wActions->setAutoFillBackground(true);
	wActions->setFixedSize(450, 60);
	wActions->setLayout(lActions);
	
	QLabel *wTable = new QLabel(this);
	wTable->setLayout(grid_seats);
	//wTable->setBackgroundRole(QPalette::Base);
	wTable->setScaledContents(true);
	QImage image("gfx/table/default.png");
	wTable->setPixmap(QPixmap::fromImage(image));
	/////
	
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(wTable, 90);
	layout->addWidget(wActions, 1, Qt::AlignCenter);
	
	setLayout(layout);
}

void WTable::updateView()
{
	int my_cid = ((PClient*)qApp)->getMyCId();
	
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(gid, tid);
	
	if (!tinfo)
		return;
	
	table_snapshot *snap = &(tinfo->snap);
	
	for (unsigned int i=0; i < 10; i++)
	{
		seatinfo *seat = &(snap->seats[i]);
		
		if (seat->valid)
		{
			int cid = seat->client_id;
			playerinfo *pinfo = ((PClient*)qApp)->getPlayerInfo(cid);
			
			if (pinfo)
				wseats[i]->setName(pinfo->name);
			else
				wseats[i]->setName("???");
			
			wseats[i]->setStake(seat->stake);
			
			if (seat->in_round)
			{
				wseats[i]->setAction(Player::Check /* FIXME */, seat->bet);
				
				if (my_cid == cid)
				{
					vector<Card> allcards;
					tinfo->holecards.copyCards(&allcards);
					
					if (allcards.size() == 2)
					{
						char card1[3], card2[3];
						strcpy(card1, allcards[0].getName());
						strcpy(card2, allcards[1].getName());
						wseats[i]->setCards(card1, card2);
					}
					else
						wseats[i]->setCards("blank", "blank");
				}
				else
				{
					vector<Card> allcards;
					seat->holecards.copyCards(&allcards);
					
					if (allcards.size())
					{
						char card1[3], card2[3];
						strcpy(card1, allcards[0].getName());
						strcpy(card2, allcards[1].getName());
						wseats[i]->setCards(card1, card2);
					}
					else
						wseats[i]->setCards("back", "back");
				}
			}
			else
			{
				wseats[i]->setAction(Player::Fold, 0.0f);
				wseats[i]->setCards("blank", "blank");
			}
			
			if (i == snap->s_cur)
				wseats[i]->setCurrent(true);
			else
				wseats[i]->setCurrent(false);
			
			wseats[i]->setValid(true);
		}
		else
		{
			wseats[i]->setValid(false);
		}
	}
	
	// Pots
	QString spots;
	for (unsigned int i=0; i < snap->pots.size(); i++)
	{
		char spot[128];
		snprintf(spot, sizeof(spot), "Pot %d: %.2f", i+1, snap->pots[i]);
		spots += spot;
		spots += "  ";
	}
	lblPots->setText(spots);
	
	// CommunityCards
	vector<Card> allcards;
	snap->communitycards.copyCards(&allcards);
	unsigned int cardcount = allcards.size();
	for (unsigned int i=0; i < 5; i++)
	{
		if (cardcount < i + 1)
		{
			cc[i]->loadImage("gfx/deck/default/blank.png");
		}
		else
		{
			char filename[1024];
			snprintf(filename, sizeof(filename), "gfx/deck/default/%s.png", allcards[i].getName());
			cc[i]->loadImage(filename);
		}
	}
}

void WTable::closeEvent(QCloseEvent *event)
{
	((PClient*)qApp)->wMain->addLog("table window closed");
}

void WTable::actionFold()
{
	((PClient*)qApp)->doSetAction(gid, Player::Fold);
}

void WTable::actionCheckCall()
{
	((PClient*)qApp)->doSetAction(gid, Player::Call);
}

void WTable::actionBetRaise()
{
	float amount = editAmount->text().toFloat();
	
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(gid, tid);
	
	if (!tinfo)
		return;
	
	table_snapshot *snap = &(tinfo->snap);
	
	if (snap->my_seat == -1)
		return;
	
	if (amount == snap->seats[snap->my_seat].stake + snap->seats[snap->my_seat].bet)
		((PClient*)qApp)->doSetAction(gid, Player::Allin);
	else
		((PClient*)qApp)->doSetAction(gid, Player::Raise, amount);
}

void WTable::actionShow()
{
	((PClient*)qApp)->doSetAction(gid, Player::Show);
}

void WTable::slotBetValue(int value)
{
	QString svalue;
	float max_bet = 0.0f;
	float amount;
	
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(gid, tid);
	
	if (!tinfo)
		return;
	
	table_snapshot *snap = &(tinfo->snap);
	
	if (snap->my_seat != -1)
		max_bet = snap->seats[snap->my_seat].stake + snap->seats[snap->my_seat].bet;
	
	if (!value)
		amount = 0.0f;
	else if (value == 100)
		amount = max_bet;
	else
		amount = (int)(max_bet * value / 100);
	
	svalue.setNum(amount, 'f', 2);
	editAmount->setText(svalue);
}

void WTable::slotShow()
{
	updateView();
	show();
}


WSeat::WSeat(unsigned int id, QWidget *parent) : QWidget(parent)
{
	setPalette(Qt::gray);
	setAutoFillBackground(true);
	setFixedSize(110, 140);
	
	lblCaption = new QLabel("Seat", this);
	lblCaption->setAlignment(Qt::AlignCenter);
	lblStake = new QLabel("0.00", this);
	lblStake->setAlignment(Qt::AlignCenter);
	lblAction = new QLabel("Action", this);
	lblAction->setAlignment(Qt::AlignCenter);
	
	///////
	const int sx = 45;
	const int sy = 60;
	card1 = new WPicture("gfx/deck/default/back.png", this);
	card1->setFixedSize(sx, sy);
	card2 = new WPicture("gfx/deck/default/back.png", this);
	card2->setFixedSize(sx, sy);
	
	QHBoxLayout *lCards = new QHBoxLayout();
	lCards->addWidget(card1);
	lCards->addWidget(card2);
	
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(lblCaption);
	layout->addLayout(lCards);
	layout->addWidget(lblStake);
	layout->addWidget(lblAction);
	setLayout(layout);
}

void WSeat::setValid(bool valid)
{
	setAutoFillBackground(valid);
	
	lblCaption->setVisible(valid);
	lblStake->setVisible(valid);
	lblAction->setVisible(valid);
	card1->setVisible(valid);
	card2->setVisible(valid);
}

void WSeat::setName(QString name)
{
	lblCaption->setText(name);
}

void WSeat::setStake(float amount)
{
	QString samount;
	samount.setNum(amount, 'f', 2);
	lblStake->setText(samount);
}

void WSeat::setAction(Player::PlayerAction action, float amount)
{
	QString samount;
	samount.setNum(amount, 'f', 2);
	
	if (action == Player::Fold)
		lblAction->setText("Folded");
	else
		lblAction->setText(samount);
}

void WSeat::setCurrent(bool cur)
{
	if (cur)
		setPalette(Qt::green);
	else
		setPalette(Qt::gray);
}

void WSeat::setCards(const char *c1, const char *c2)
{
	char filename[1024];
	
	snprintf(filename, sizeof(filename), "gfx/deck/default/%s.png", c1);
	card1->loadImage(filename);
	
	snprintf(filename, sizeof(filename), "gfx/deck/default/%s.png", c2);
	card2->loadImage(filename);
}

WPicture::WPicture(const char *filename, QWidget *parent) : QLabel(parent)
{
	setBackgroundRole(QPalette::Base);
	//setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	//sizePolicy().hasHeightForWidth();
	setScaledContents(true);
	
	loadImage(filename);
}

void WPicture::loadImage(const char *filename)
{
	QImage image(filename);
	setPixmap(QPixmap::fromImage(image));
}

/*
int WPicture::heightForWidth ( int w )
{
	return -1;
}
*/
