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
#include <cmath>

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
#include <QTimeLine>
#include <QGraphicsItemAnimation>

#include "Config.h"
#include "Debug.h"

#include "pclient.hpp"

#include "WTable.hpp"
#include "ChatBox.hpp"
#include "Seat.hpp"
#include "DealerButton.hpp"

using namespace std;

#ifdef DEBUG
#	include <QDebug>
	
QDebug operator << (QDebug s, const Player::PlayerAction& p)		
{
	switch (p)
	{
		case Player::None:
				s << "None";
			break;
		case Player::ResetAction:
				s << "ResetAction";
			break;
		case Player::Check:
				s << "Check";
			break;
		case Player::Fold:
				s << "Fold";
			break;
		case Player::Call:
				s << "Call";
			break;
		case Player::Bet:
				s << "Bet";
			break;
		case Player::Raise:
				s << "Raise";
			break;
		case Player::Allin:
				s << "Allin";
			break;
		case Player::Show:
				s << "Show";
			break;
		case Player::Muck:
				s << "Muck";
			break;
		case Player::Sitout:
				s << "Sitout";
			break;
		case Player::Back:
				s << "Back";
			break;
		default:
			s << "unkown";
	}
	return s;
}

QDebug operator << (QDebug s, const seatinfo& si)	
{
	s << "seatinfo" << "\n";
	s << "\t client_id= " << si.client_id << "\n";
	s << "\t valid= " << (si.valid ? "true" : "false") << "\n";
	s << "\t in_round= " << (si.in_round ? "true" : "false") << "\n";
	s << "\t PlayerAction= " << si.action << "\n";
	s << "\t bet= " << si.bet << "\n";
	s << "\t stake= " << si.stake << "\n";
	s << "end seatinfo" << "\n";
	
	return s;
}

QDebug operator << (QDebug s, const table_snapshot& t)
{
	s << "table_snapshot\n";
	s << "\t state= " << t.state << "\n";
	s << "\t betting_round= " << t.betting_round << "\n";
	s << "\t s_dealer= " << t.s_dealer << "\n";
	s << "\t small blind= " << t.s_sb << "\n";
	s << "\t big blind= " << t.s_bb << "\n";
	s << "\t s_cur= " << t.s_cur << "\n";
	s << "\t s_lastbet= " << t.s_lastbet << "\n";
	s << "\t minimum_bet= " << t.minimum_bet << "\n";
	s << "\t my_seat= " << t.my_seat << "\n";
	s << "end table_snapshot" << "\n";
	
	return s;
}

#endif /* DEBUG */

qreal normalize(QPointF& vector)
{
	const qreal len = std::sqrt(
		vector.x() * vector.x() +
		vector.y() * vector.y());

	if(len != 0)
	{
		const qreal InvLen = 1 / len;

		vector *= InvLen;
	}
	return len;
}

const unsigned int WTable::nMaxSeats = 10;

WTable::WTable(int gid, int tid, QWidget *parent)
:	QGraphicsView(parent),
	m_nGid(gid),
	m_nTid(tid),
	m_pImgTable(0)
{
	QDir::setCurrent("C:/Eigene Dateien/cpp/holdingnuts/trunk/data");
	
	// TODO: andere lösung muss her !!!!
	QImage imgTable("gfx/table/default.png");
	imgTable = imgTable.scaled(imgTable.width() / 2, imgTable.height() / 2);

	// scene
	QGraphicsScene* pScene = new QGraphicsScene(
		0, 0, imgTable.width(), 700, this);

	pScene->setBackgroundBrush(Qt::black);//QPixmap("gfx/table/background.png"));
	
	m_pImgTable = pScene->addPixmap(QPixmap::fromImage(imgTable));

	m_pDealerButton = new DealerButton;
	m_pDealerButton->scale(0.5, 0.5);
	m_pDealerButton->setPos(
		imgTable.width() / 5, imgTable.height() / 2);

	pScene->addItem(m_pDealerButton);

	// view
	this->setScene(pScene);
	this->setRenderHint(QPainter::SmoothPixmapTransform); // QPainter::Antialiasing
	this->setCacheMode(QGraphicsView::CacheBackground);
	this->setMinimumSize(
		static_cast<int>(pScene->width()),
		static_cast<int>(pScene->height()));
	this->setWindowTitle(tr("HoldingNuts table"));
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// ui - widgets
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
	
	m_pChat	= new ChatBox("", m_nGid, m_nTid, ChatBox::INPUTLINE_BOTTOM, 120, this);
	m_pChat->show();
	
	////////
	
	wCC = new QWidget(this);
	wCC->setFixedSize(300, 80);  // FIXME
	
	QHBoxLayout *lCC = new QHBoxLayout();
	for (unsigned int i=0; i < 5; i++)
	{
		cc[i] = new WPicture("gfx/deck/default/back.png", this);
//		cc[i]->setFixedSize(45, 60);
		lCC->addWidget(cc[i]);
	}
	
	wCC->setLayout(lCC);
	
	///////
	
	for (unsigned int i = 0; i < nMaxSeats; i++)
	{
		wseats[i] = new Seat(i, this);
		wseats[i]->scale(0.5, 0.5);	// TODO: remove
		
		wseats[i]->setPos(calcSeatPos(i));

		pScene->addItem(wseats[i]);
	}
	
	////////
	
	lblPots = new QLabel("Pots", this);
	lblPots->setAlignment(Qt::AlignCenter);
}

QPointF WTable::calcSeatPos(unsigned int nSeatID)
{
	Q_ASSERT_X(nSeatID < nMaxSeats, __func__, "invalided Seat Number");

	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3

	// TODO: note size from seat images
	// TODO: calc seat position
	const int height_start = 80;

	switch (nSeatID)
	{
		case 0:
			return QPointF(600, height_start);
		case 1:
			return QPointF(750, 190);
		case 2:
			return QPointF(750, 340);
		case 3:
			return QPointF(600, 450);
		case 4:
			return QPointF(400, 450);
		case 5:
			return QPointF(200, 450);
		case 6:
			return QPointF(50, 340);
		case 7:
			return QPointF(50, 190);
		case 8:
			return QPointF(200, height_start);
		case 9:
			return QPointF(400, height_start);
	}
	return QPointF(0, 0);
}

QPointF WTable::calcDealerBtnPos(unsigned int nSeatID)
{
	Q_ASSERT_X(nSeatID < nMaxSeats, __func__, "invalided Seat Number");
	
	const QPointF& ptBase = wseats[nSeatID]->pos() + wseats[nSeatID]->boundingRect().center();
	const QPointF& ptMid = m_pImgTable->sceneBoundingRect().center();
	
	QPointF vDir = ptBase - ptMid;

	normalize(vDir);

	return ptBase - (vDir * 40);
}

void WTable::updateView()
{
	// TODO: wait for server
	
	int my_cid = ((PClient*)qApp)->getMyCId();
	
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	Q_ASSERT_X(tinfo, __func__, "getTableInfo failed");

	table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, __func__, "invalid snapshot pointer");

	qDebug() << *snap;

	for (unsigned int i=0; i < nMaxSeats; i++)
	{
		seatinfo *seat = &(snap->seats[i]);
		
//		qDebug() << "seatinfo from " << i << "\n" << *seat;
		
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
				wseats[i]->setAction(seat->action, seat->bet);
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
				wseats[i]->setAction(Player::None);
				wseats[i]->setCards("blank", "blank");
			}

//			this->scene()->update(wseats[i]->boundingRect());
			
		}
		else
			wseats[i]->setValid(false);
	}
	
	// dealerbutton
	if (snap->state == Table::Blinds) // TODO: NewRound
	{
		QTimeLine *timer = new QTimeLine(2000);
		timer->setFrameRange(0, 100);

		QGraphicsItemAnimation *anim = new QGraphicsItemAnimation;

		anim->setItem(m_pDealerButton);
		anim->setTimeLine(timer);

		const qreal steps = 100;

		QPointF ptStep = (m_pDealerButton->scenePos() - calcDealerBtnPos(snap->s_dealer)) / steps;
		
		for (int i = 0; i < static_cast<int>(steps); ++i)
			anim->setPosAt(
				i / steps,
				QPointF(m_pDealerButton->scenePos() - (ptStep * i)));

		timer->start();
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

	this->viewport()->update();	// TODO: remove
}

void WTable::addChat(const QString& from, const QString& text)
{
	m_pChat->addMessage(from, text);
}

void WTable::addServerMessage(const QString& text)
{
	m_pChat->addMessage(text, Qt::blue);
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
	const QSize& size = event->size();

	this->scene()->setSceneRect(
		QRectF(0, 0, size.width(), size.height()));

	if (event->oldSize().isValid())
	{
		const qreal ratio_x = 
			static_cast<qreal>(event->size().width()) / 
			static_cast<qreal>(event->oldSize().width());
		const qreal ratio_y = 
			static_cast<qreal>(event->size().height()) / 
			static_cast<qreal>(event->oldSize().height());

		m_pImgTable->scale(ratio_x, ratio_y);
		
		for (unsigned int i = 0; i < nMaxSeats; ++i)
		{
			wseats[i]->setPos(
				wseats[i]->pos().x() * ratio_x,
				wseats[i]->pos().y() * ratio_y);
			wseats[i]->scale(ratio_x, ratio_y);
		}
		
		m_pDealerButton->setPos(
			m_pDealerButton->pos().x() * ratio_x,
			m_pDealerButton->pos().y() * ratio_y);
	}	

	m_pChat->move(20, size.height() - m_pChat->height() - 10);

	m_LayoutActions->move(
		m_pChat->width() + ((size.width() - m_pChat->width() - m_LayoutActions->width()) / 2),
		size.height() - (m_pChat->height() / 2) - (m_LayoutActions->height() / 2));

	// community-cards
	wCC->move(
		this->width() / 2 - wCC->width() / 2,
		this->height() / 2 - wCC->height() / 2);
//		/* offset_x + */ twidth/2 - wCC->width() /2,
//		offset_y + theight/2 - wCC->height() /2);
	
	// pots
//	lblPots->move(offset_x + twidth/2 - lblPots->width() /2, wCC->y() + wCC->height() + 10);
}

