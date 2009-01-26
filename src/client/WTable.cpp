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


#include <cstdio>

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QTime>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include "Config.h"
#include "Debug.h"

#include "pclient.hpp"

#include "WTable.hpp"
#include "ChatBox.hpp"
#include "Seat.hpp"
#include "DealerButton.hpp"

using namespace std;

const unsigned int WTable::nMaxSeats = 10;

WTable::WTable(int gid, int tid, QWidget *parent) : QGraphicsView(parent), m_nGid(gid), m_nTid(tid)
{
	QDir::setCurrent("C:/Eigene Dateien/cpp/holdingnuts/trunk/data");
	
	// TODO: andere lösung muss her !!!!
	QImage imgTable("gfx/table/default.png");
	imgTable = imgTable.scaled(imgTable.width() / 2, imgTable.height() / 2);

	// scene
	QGraphicsScene* pScene = new QGraphicsScene(
		0, 0, imgTable.width(), 700, this);

	pScene->addPixmap(QPixmap::fromImage(imgTable));

	m_pDealerButton = new DealerButton;
	m_pDealerButton->scale(0.5, 0.5);
	m_pDealerButton->setPos(150, 150);

	pScene->addItem(m_pDealerButton);

///////////////////////////////
	
	//setAttribute(Qt::WA_DeleteOnClose); // FIXME: implement correctly

	// view
	this->setScene(pScene);
	this->setRenderHint(QPainter::SmoothPixmapTransform); // QPainter::Antialiasing
	this->setCacheMode(QGraphicsView::CacheBackground);
	this->setMinimumSize(
		static_cast<int>(pScene->width()),
		static_cast<int>(pScene->height()));
	this->setBackgroundBrush(Qt::black);//QPixmap("gfx/table/background.png"));
	this->setWindowTitle(tr("HoldingNuts table"));
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// ui - widgets

	// set window-background
//	QImage bgimage("gfx/table/background.png");
//	setPixmap(QPixmap::fromImage(bgimage));
//	setScaledContents(true);
	
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
	
	
	
//	QLabel *wActions 
	m_LayoutActions = new QLabel(this);
	//wActions->setPalette(Qt::gray);
	//wActions->setAutoFillBackground(true);
	QImage aimage("gfx/table/actions.png");
	m_LayoutActions->setPixmap(QPixmap::fromImage(aimage));
	m_LayoutActions->setScaledContents(true);
	m_LayoutActions->setFixedSize(400, 60);
	m_LayoutActions->setLayout(stlayActions);
	
//	wTable = new QLabel(this);
	//wTable->setBackgroundRole(QPalette::Base);
	//wTable->setMinimumSize(300, 300);
//	wTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//	wTable->setScaledContents(true);

//	QImage image("gfx/table/default.png");
//	wTable->setPixmap(QPixmap::fromImage(image));
	/////
	
	m_pChat	= new ChatBox("", m_nGid, m_nTid, ChatBox::INPUTLINE_BOTTOM, 120, this);
	m_pChat->show();
	
//	QHBoxLayout* actionHBox = new QHBoxLayout();

//	actionHBox->addWidget(m_pChat);
//	actionHBox->addWidget(wActions, 1, Qt::AlignCenter);
	
//	QWidget *wSpacer = new QWidget(this);
//	wSpacer->setFixedHeight(50);
	
//	QVBoxLayout *layout = new QVBoxLayout();
//	layout->addWidget(wSpacer);
//	layout->addWidget(wTable, 90);
//	layout->addLayout(actionHBox);
	
	
//	setLayout(layout);
	
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
	
	for (unsigned int i = 0; i < nMaxSeats; i++)
	{
		wseats[i] = new Seat(i, this);
		
		wseats[i]->scale(0.5, 0.5);

		pScene->addItem(wseats[i]);
	}
	
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

	int offset_x = 80;
	int offset_y = 50;
	int twidth = 500;
	int theight = 300;
//	int offset_x = wTable->x();
//	int offset_y = wTable->y();
//	int twidth = wTable->width();
//	int theight = wTable->height();
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
		/*
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
		wseats[i]->card2->move(cx + 10, cy);*/
	}
	
	
	// community-cards
	wCC->move(
		offset_x + twidth/2 - wCC->width() /2,
		offset_y + theight/2 - wCC->height() /2);
	
	// pots
	lblPots->move(offset_x + twidth/2 - lblPots->width() /2, wCC->y() + wCC->height() + 10);
	
}

QPointF WTable::calcSeatPos(unsigned int nSeatID)
{
	Q_ASSERT_X(nSeatID < nMaxSeats, __func__, "invalided Seat Number");

	//		9	  dealer	0
	//	 8					  1
	// 7						2
	//   6					  3
	//		5				4

	// TODO: note size from seat images
	// TODO: calc seat position
	const int width_seat = 125;
	const int height_start = 50;
	const int height_step = 65;

	switch (nSeatID)
	{
		case 0:
			return QPointF(width() - width_seat - 150, height_start);
		case 1:
			return QPointF(width() - width_seat - 60, height_start + height_step - 10);
		case 2:
			return QPointF(width() - width_seat - 10, height_start + height_step * 2 + 5);
		case 3:
			return QPointF(width() - width_seat - 60, height_start + height_step * 3 + 20);
		case 4:
			return QPointF(width() - width_seat - 175, height_start + height_step * 4 - 20);
		case 5:
			return QPointF(175, height_start + height_step * 4 - 20);
		case 6:
			return QPointF(60, height_start + height_step * 3 + 20);
		case 7:
			return QPointF(10, height_start + height_step * 2 + 5);
		case 8:
			return QPointF(60, height_start + height_step - 10);
		case 9:
			return QPointF(150, height_start);
	}

	return QPointF(0, 0);
}

void WTable::updateView()
{
	int my_cid = ((PClient*)qApp)->getMyCId();
	
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	Q_ASSERT_X(tinfo, __func__, "getTableInfo failed");

	table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, __func__, "invalid snapshot pointer");
	
	for (unsigned int i=0; i < nMaxSeats; i++)
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
			wseats[i]->setValid(seat->in_round);
			wseats[i]->setCurrent(snap->s_cur == i);
			
			if (seat->in_round)
			{
				wseats[i]->setAction(Player::Check /* FIXME */, seat->bet);
				wseats[i]->setMySeat(my_cid == cid);
/*
				std::vector<Card> allcards;
				
				if (my_cid == cid)
					tinfo->holecards.copyCards(&allcards);
				else
					seat->holecards.copyCards(&allcards);

				if (allcards.size() == 2)
					wseats[i]->setCards(
						allcards[0].getName(),
						allcards[1].getName());
				else
					wseats[i]->setCards("blank", "blank");
*/					
				if (my_cid == cid)
				{
					std::vector<Card> allcards;
					tinfo->holecards.copyCards(&allcards);
					
					if (allcards.size() == 2)
					{
						char card1[3], card2[3];
						strcpy(card1, allcards[0].getName());
						strcpy(card2, allcards[1].getName());
						wseats[i]->setCards(allcards[0].getName(), allcards[1].getName());
					}
					else
						wseats[i]->setCards("blank", "blank");
					
//					wseats[i]->scard1->setVisible(false);
//					wseats[i]->scard2->setVisible(false);

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
						wseats[i]->card1->setVisible(true);
						wseats[i]->card2->setVisible(true);
					}
					else
					{
						wseats[i]->setCards("back", "back");
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
			snap->state == Table::AllFolded ||
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

void WTable::addChat(const QString& from, const QString& text)
{
	m_pChat->addMessage(from, text);
}

void WTable::addServerMessage(const QString& text)
{
	m_pChat->addMessage(text, Qt::red);
}

void WTable::closeEvent(QCloseEvent *event)
{
	((PClient*)qApp)->wMain->addLog("table window closed");
}

void WTable::actionFold()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Fold);
}

void WTable::actionCheckCall()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Call);
}

void WTable::actionBetRaise()
{
	float amount = editAmount->text().toFloat();
	
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	if (!tinfo)
		return;
	
	table_snapshot *snap = &(tinfo->snap);
	
	if (snap->my_seat == -1)
		return;
	
	if (amount == snap->seats[snap->my_seat].stake + snap->seats[snap->my_seat].bet)
		((PClient*)qApp)->doSetAction(m_nGid, Player::Allin);
	else
		((PClient*)qApp)->doSetAction(m_nGid, Player::Raise, amount);
	
	// reset the amount-slider
	sliderAmount->setValue(0);
}

void WTable::actionShow()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Show);
}

void WTable::actionMuck()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Muck);
}

void WTable::slotBetValue(int value)
{
	QString svalue;
	float max_bet = 0.0f;
	float min_bet;
	float amount;
	
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
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

void WTable::resizeEvent(QResizeEvent *event)
{
	arrangeItems();
	
	const QSize size = event->size();

	m_pChat->move(20, size.height() - m_pChat->height() - 10);

	m_LayoutActions->move(
		m_pChat->width() + ((size.width() - m_pChat->width() - m_LayoutActions->width()) / 2),
		size.height() - (m_pChat->height() / 2) - (m_LayoutActions->height() / 2));
}

