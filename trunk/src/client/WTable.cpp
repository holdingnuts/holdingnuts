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


WTable::WTable(int gid, int tid, QWidget *parent) : QLabel(parent)
{
	this->gid = gid;
	this->tid = tid;
	
	//setAttribute(Qt::WA_DeleteOnClose); // FIXME: implement correctly
	
	setWindowTitle("HoldingNuts table");
	setMinimumSize(520, 450);
	
	// set window-background
	QImage bgimage("gfx/table/background.png");
	setPixmap(QPixmap::fromImage(bgimage));
	setScaledContents(true);
	
	///////
	
	QPushButton *btnFold = new QPushButton(tr("Fold"), this);
	connect(btnFold, SIGNAL(clicked()), this, SLOT(actionFold()));
	
	QPushButton *btnCheckCall = new QPushButton(tr("Check/Call"), this);
	connect(btnCheckCall, SIGNAL(clicked()), this, SLOT(actionCheckCall()));
	
	QPushButton *btnBetRaise = new QPushButton(tr("Bet/Raise"), this);
	connect(btnBetRaise, SIGNAL(clicked()), this, SLOT(actionBetRaise()));
	
	QPushButton *btnShow = new QPushButton(tr("Show"), this);
	connect(btnShow, SIGNAL(clicked()), this, SLOT(actionShow()));
	
	QPushButton *btnMuck = new QPushButton(tr("Muck"), this);
	connect(btnMuck, SIGNAL(clicked()), this, SLOT(actionMuck()));
	
	
	QCheckBox *chkFold = new QCheckBox(tr("Fold"), this);
	
	
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
	
	QHBoxLayout *lActions = new QHBoxLayout();
	lActions->addWidget(btnFold);
	lActions->addWidget(btnCheckCall);
	lActions->addWidget(btnBetRaise);
	lActions->addLayout(lAmount);
	
	QHBoxLayout *lPreActions = new QHBoxLayout();
	lPreActions->addWidget(chkFold);
	
	QHBoxLayout *lPostActions = new QHBoxLayout();
	lPostActions->addWidget(btnMuck);
	lPostActions->addWidget(btnShow);
	
	
	stlayActions = new QStackedLayout();
	QWidget *pageActions = new QWidget(this);
	QWidget *pagePreActions = new QWidget(this);
	QWidget *pagePostActions = new QWidget(this);
	QWidget *pageNoActions = new QWidget(this);
	
	pageActions->setLayout(lActions);
	pagePreActions->setLayout(lPreActions);
	pagePostActions->setLayout(lPostActions);
	
	stlayActions->addWidget(pageActions);
	stlayActions->addWidget(pagePreActions);
	stlayActions->addWidget(pagePostActions);
	stlayActions->addWidget(pageNoActions);
	
	
	
	QLabel *wActions = new QLabel(this);
	//wActions->setPalette(Qt::gray);
	//wActions->setAutoFillBackground(true);
	QImage aimage("gfx/table/actions.png");
	wActions->setPixmap(QPixmap::fromImage(aimage));
	wActions->setScaledContents(true);
	wActions->setFixedSize(400, 60);
	wActions->setLayout(stlayActions);
	
	wTable = new QLabel(this);
	//wTable->setBackgroundRole(QPalette::Base);
	//wTable->setMinimumSize(300, 300);
	wTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	wTable->setScaledContents(true);
	QImage image("gfx/table/default.png");
	wTable->setPixmap(QPixmap::fromImage(image));
	/////
	
	QWidget *wSpacer = new QWidget(this);
	wSpacer->setFixedHeight(50);
	
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(wSpacer);
	layout->addWidget(wTable, 90);
	layout->addWidget(wActions, 1, Qt::AlignCenter);
	
	setLayout(layout);
	
	
	////////
	
	wCC = new QWidget(this);
	wCC->setFixedSize(300, 80);  // FIXME
	
	QHBoxLayout *lCC = new QHBoxLayout();
	for (unsigned int i=0; i < 5; i++)
	{
		cc[i] = new WPicture("gfx/deck/default/back.png", this);
		cc[i]->setFixedSize(45, 60);
		lCC->addWidget(cc[i]);
	}
	
	wCC->setLayout(lCC);
	
	///////
	
	const int seatcount = 10;
	for (int i=0; i < seatcount; i++)
		wseats[i] = new WSeat(i, this);
	
	////////
	
	lblPots = new QLabel("Pots", this);
	lblPots->setAlignment(Qt::AlignCenter);
	
	////////
	
	// update the window
	arrangeItems();
}

void WTable::arrangeItems()
{
	enum splacement { SRight, SBelow, SLeft, SAbove };
	
	struct seatinfo {
		int col;  // x
		int row;  // y
		splacement place;
	} seats[] = {
		// col  row     placement
		{ 7,	0,	SBelow },	// seat 1
		{ 9,	1,	SLeft },	// seat 2
		{ 10,	2,	SLeft },	// seat 3
		{ 9,	3,	SLeft },	// seat 4
		{ 7,	4,	SAbove },	// seat 5
		{ 3,	4,	SAbove },	// seat 6
		{ 1,	3,	SRight },	// seat 7
		{ 0,	2,	SRight },	// seat 8
		{ 1,	1,	SRight },	// seat 9
		{ 3,	0,	SBelow }	// seat 10
	};
	
	int offset_x = wTable->x();
	int offset_y = wTable->y();
	int twidth = wTable->width();
	int theight = wTable->height();
	int cols_num = 11;
	int rows_num = 5;
	int cell_w = twidth / cols_num;
	int cell_h = theight / rows_num;
	
	
	const int seatcount = sizeof(seats) / sizeof(seats[0]);
	
	for (int i=0; i < seatcount; i++)
	{
		wseats[i]->move(
			offset_x + seats[i].col * cell_w + cell_w / 2 - wseats[i]->width() / 2,
			offset_y + seats[i].row * cell_h + cell_h / 2 - wseats[i]->height() / 2);
		
		
		int cw = wseats[i]->scard1->width();
		int ch = wseats[i]->scard1->height();
		int cx, cy;
		
		if (seats[i].place == SLeft || seats[i].place == SRight)
		{
			cx = wseats[i]->x() + (seats[i].place == SLeft ? -cw : wseats[i]->width());
			cy = wseats[i]->y();
	
		}
		else if (seats[i].place == SBelow ||seats[i].place == SAbove)
		{
			cx = wseats[i]->x();
			cy = wseats[i]->y() + (seats[i].place == SBelow ? wseats[i]->height() : -ch);
		}
		
		wseats[i]->scard1->move(cx, cy);
		wseats[i]->scard2->move(cx + 5, cy);
		
		////
		cx = wseats[i]->x();
		cy = wseats[i]->y() - wseats[i]->card1->height();
		wseats[i]->card1->move(cx, cy);
		wseats[i]->card2->move(cx + 10, cy);
	}
	
	
	// community-cards
	wCC->move(offset_x + twidth/2 - wCC->width() /2, offset_y + theight/2 - wCC->height() /2);
	
	// pots
	lblPots->move(offset_x + twidth/2 - lblPots->width() /2, wCC->y() + wCC->height() + 10);
	
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
					
					wseats[i]->scard1->setVisible(false);
					wseats[i]->scard2->setVisible(false);
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
					
					if (allcards.size() && (snap->state == Table::Showdown || snap->state == Table::EndRound))
					{
						wseats[i]->card1->setVisible(true);
						wseats[i]->card2->setVisible(true);
					}
					else
					{
						wseats[i]->card1->setVisible(false);
						wseats[i]->card2->setVisible(false);
					}
				}
			}
			else
			{
				wseats[i]->setAction(Player::Fold);
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
			wseats[i]->card1->setVisible(false);
			wseats[i]->card2->setVisible(false);
			wseats[i]->scard1->setVisible(false);
			wseats[i]->scard2->setVisible(false);
			
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
	
	
	if (snap->my_seat != -1)
	{
		// minimum bet
		QString svalue;
		if (snap->minimum_bet > snap->seats[snap->my_seat].stake)
			svalue.setNum(snap->seats[snap->my_seat].stake, 'f', 2);
		else
			svalue.setNum(snap->minimum_bet, 'f', 2);
		editAmount->setText(svalue);
		
		// show correct actions
		if (!snap->seats[snap->my_seat].in_round ||
			snap->state == Table::Showdown ||
			snap->state == Table::EndRound)
		{
			stlayActions->setCurrentIndex(3);
		}
		else
		{
			if (snap->state == Table::AskShow)
			{
				if ((int)snap->s_cur == snap->my_seat)
					stlayActions->setCurrentIndex(2);
				else
					stlayActions->setCurrentIndex(3);
			}
			else
			{
				if ((int)snap->s_cur == snap->my_seat)
					stlayActions->setCurrentIndex(0);
				else
					stlayActions->setCurrentIndex(1);
			}
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

void WTable::actionMuck()
{
	((PClient*)qApp)->doSetAction(gid, Player::Muck);
}

void WTable::slotBetValue(int value)
{
	QString svalue;
	float max_bet = 0.0f;
	float min_bet;
	float amount;
	
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(gid, tid);
	
	if (!tinfo)
		return;
	
	table_snapshot *snap = &(tinfo->snap);
	
	if (snap->my_seat != -1)
	{
		max_bet = snap->seats[snap->my_seat].stake + snap->seats[snap->my_seat].bet;
		min_bet = snap->minimum_bet;
	}
	
	if (!value)
		amount = 0.0f;
	else if (value == 100)
		amount = max_bet;
	else
	{
		if (value <= 50)  // wieder range for small bets (half slider = 25% of stake)
			amount = (int)((max_bet * 25 / 100) * value * 2 / 100);
		else
			amount = (int)(max_bet * value / 100);
	}
	
	amount += min_bet;
	if (amount > max_bet)
		amount = max_bet;
	
	svalue.setNum(amount, 'f', 2);
	editAmount->setText(svalue);
}

void WTable::slotShow()
{
	updateView();
	show();
}

void WTable::slotTest()
{
	QWidget *w = cc[0];
	w->move(w->x()+10, w->y()+10);
	
	QTimer::singleShot(100, this, SLOT(slotTest()));
}

void WTable::resizeEvent(QResizeEvent * event)
{
	arrangeItems();
}

WSeat::WSeat(unsigned int id, QWidget *parent) : QLabel(parent)
{
	//setPalette(Qt::gray);
	//setAutoFillBackground(true);
	setFixedSize(100, 70);
	
	QImage image("gfx/table/seat.png");
	setPixmap(QPixmap::fromImage(image));
	setScaledContents(true);
	
	lblCaption = new QLabel("Seat", this);
	lblCaption->setAlignment(Qt::AlignCenter);
	lblStake = new QLabel("0.00", this);
	lblStake->setAlignment(Qt::AlignCenter);
	lblAction = new QLabel("Action", this);
	lblAction->setAlignment(Qt::AlignCenter);
	
	///////
	int sx = 45;
	int sy = 60;
	card1 = new WPicture("gfx/deck/default/back.png", parent);
	card1->setFixedSize(sx, sy);
	card2 = new WPicture("gfx/deck/default/back.png", parent);
	card2->setFixedSize(sx, sy);
	
	// miniature cards
	sx = 17;
	sy = 25;
	scard1 = new WPicture("gfx/deck/default/back.png", parent);
	scard1->setFixedSize(sx, sy);
	scard2 = new WPicture("gfx/deck/default/back.png", parent);
	scard2->setFixedSize(sx, sy);
	
	//QHBoxLayout *lCards = new QHBoxLayout();
	//lCards->addWidget(card1);
	//lCards->addWidget(card2);
	
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(lblCaption);
	//layout->addLayout(lCards);
	layout->addWidget(lblStake);
	layout->addWidget(lblAction);
	setLayout(layout);
}

void WSeat::setValid(bool valid)
{
	lblCaption->setVisible(valid);
	lblStake->setVisible(valid);
	lblAction->setVisible(valid);
	/*
	card1->setVisible(valid);
	card2->setVisible(valid);
	scard1->setVisible(valid);
	scard2->setVisible(valid);*/
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
	QImage image;
	
	if (cur)
		image.load("gfx/table/seat_current.png");
	else
		image.load("gfx/table/seat.png");
	
	setPixmap(QPixmap::fromImage(image));
}

void WSeat::setCards(const char *c1, const char *c2)
{
	char filename[1024];
	
	snprintf(filename, sizeof(filename), "gfx/deck/default/%s.png", c1);
	card1->loadImage(filename);
	scard1->loadImage(filename);
	
	snprintf(filename, sizeof(filename), "gfx/deck/default/%s.png", c2);
	card2->loadImage(filename);
	scard2->loadImage(filename);
}

WPicture::WPicture(const char *filename, QWidget *parent) : QLabel(parent)
{
	//setBackgroundRole(QPalette::Base);
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
