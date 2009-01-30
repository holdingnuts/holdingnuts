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
#include <numeric>

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
#include "Logger.h"
#include "ConfigParser.hpp"
#include "GameLogic.hpp"

#include "pclient.hpp"

#include "WTable.hpp"
#include "ChatBox.hpp"
#include "Seat.hpp"
#include "DealerButton.hpp"
#include "EditableSlider.hpp"

using namespace std;

extern ConfigParser config;

#ifdef DEBUG
#	include <QDebug>

QDebug operator << (QDebug s, const Table::State& state)
{
	switch (state)
	{
		case Table::GameStart:
				s << "GameStart";
			break;
		case Table::ElectDealer:
				s << "ElectDealer";
			break;
		case Table::NewRound:
				s << "NewRound";
			break;
		case Table::Blinds:
				s << "Blinds";
			break;
		case Table::Betting:
				s << "Betting";
			break;
		case Table::AskShow:
				s << "AskShow";
			break;
		case Table::AllFolded:
				s << "AllFolded";
			break;
		case Table::Showdown:
				s << "Showdown";
			break;
		case Table::EndRound:
				s << "EndRound";
			break;
		default:
			s << static_cast<int>(state);
	}
	return s;
}

QDebug operator << (QDebug s, const Table::BettingRound& r)
{
	switch (r)
	{
		case Table::Preflop:
				s << "Preflop";
			break;
		case Table::Flop:
				s << "Flop";
			break;
		case Table::Turn:
				s << "Turn";
			break;
		case Table::River:
				s << "River";
			break;
		default:
			s << static_cast<int>(r);
	}
	return s;
}

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
	s << "\t state= " << Table::State(t.state) << "\n";
	s << "\t betting_round= " << Table::BettingRound(t.betting_round) << "\n";
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
	// scene
	QGraphicsScene* pScene = new QGraphicsScene(0, 0, 900, 700, this);

//	pScene->setBackgroundBrush(Qt::black);
	// don't use bsptree
	pScene->setItemIndexMethod(QGraphicsScene::NoIndex);

	QImage imgTable("gfx/table/table.png");
	
	m_pImgTable = pScene->addPixmap(QPixmap::fromImage(imgTable));
	m_pImgTable->scale(
		pScene->width() / imgTable.width(),
		(pScene->height() - 150) / imgTable.height()); // 150 == height chatbox

	m_pDealerButton = new DealerButton;
	m_pDealerButton->scale(0.5, 0.5);
	m_pDealerButton->setPos(
		pScene->width() / 5, pScene->height() / 2);

	pScene->addItem(m_pDealerButton);

	// view
	this->setScene(pScene);
//	this->setRenderHint(QPainter::HighQualityAntialiasing);
	this->setRenderHint(QPainter::SmoothPixmapTransform);
	this->setCacheMode(QGraphicsView::CacheBackground);
	this->setMinimumSize(
		static_cast<int>(pScene->width()),
		static_cast<int>(pScene->height()));
	this->setWindowTitle(tr("HoldingNuts table"));
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

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


	m_pSliderAmount = new EditableSlider;
	
	QHBoxLayout *lActions = new QHBoxLayout();
	lActions->addWidget(btnFold);
	lActions->addWidget(btnCheckCall);
	lActions->addWidget(btnBetRaise);
	lActions->addWidget(m_pSliderAmount);
	
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
	

	m_LayoutActions = new QLabel(this);
	m_LayoutActions->setPixmap(QPixmap("gfx/table/actions.png"));
	m_LayoutActions->setScaledContents(true);
	m_LayoutActions->setFixedSize(400, 70);
	m_LayoutActions->setLayout(stlayActions);
	
	m_pChat	= new ChatBox("", m_nGid, m_nTid, ChatBox::INPUTLINE_BOTTOM, 120, this);
	m_pChat->show();
	
	////////
	
	for (unsigned int i = 0; i < nMaxSeats; i++)
	{
		wseats[i] = new Seat(i, this);
		wseats[i]->scale(0.5, 0.5);	// TODO: remove
		
		wseats[i]->setPos(calcSeatPos(i));

		pScene->addItem(wseats[i]);
	}
	
	////////
	
	lblPots = new QLabel("Pot 0: 0.00", this);
	lblPots->setFixedWidth(static_cast<int>(pScene->width()));
	lblPots->setAlignment(Qt::AlignCenter);
	
	lblHandStrength = new QLabel("HandStrength", this);
	lblHandStrength->setFixedWidth(static_cast<int>(pScene->width()));
	lblHandStrength->setAlignment(Qt::AlignCenter);
}

QPointF WTable::calcSeatPos(unsigned int nSeatID) const
{
	Q_ASSERT_X(nSeatID < nMaxSeats, __func__, "invalided Seat Number");

	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3

	// TODO: note size from seat images
	// TODO: calc seat position
	static const qreal height_start = 72;

	switch (nSeatID)
	{
		case 0:
			return QPointF(600, height_start);
		case 1:
			return QPointF(750, 190);
		case 2:
			return QPointF(750, 340);
		case 3:
			return QPointF(600, 449);
		case 4:
			return QPointF(400, 449);
		case 5:
			return QPointF(200, 449);
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

QPointF WTable::calcDealerBtnPos(unsigned int nSeatID, int distance) const
{
	Q_ASSERT_X(nSeatID < nMaxSeats, __func__, "invalided Seat Number");

	const QPointF& ptBase = wseats[nSeatID]->pos() + wseats[nSeatID]->boundingRectSeat().center();
	const QPointF& ptMid = m_pImgTable->sceneBoundingRect().center();
	
	QPointF vDir = ptBase - ptMid;

	normalize(vDir);

	return ptBase - (vDir * distance);
}

QPointF WTable::calcCCardsPos(unsigned int nCard) const
{
	Q_ASSERT_X(nCard < 5, __func__, "invalided Card Number");

	static const qreal card_spacing = 8;
	static const qreal card_width = 80;	// card_width = 72 + spacing

	return QPointF(
		((this->scene()->width() - (5 * card_width - card_spacing)) / 2) + nCard * card_width,
		275);
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
				// TODO: fold ?!?!?
				wseats[i]->setAction(seat->action);
				wseats[i]->setCards("blank", "blank");
			}

//			this->scene()->update(wseats[i]->boundingRect());
			
		}
		else
			wseats[i]->setValid(false);
	}
	
	// dealerbutton
	if (snap->state == Table::NewRound)
	{
		QTimeLine *timer = new QTimeLine(2000);
		timer->setFrameRange(0, 100);

		QGraphicsItemAnimation *anim = new QGraphicsItemAnimation;

		anim->setItem(m_pDealerButton);
		anim->setTimeLine(timer);

		const qreal steps = 100;
		const QPointF ptStep = (m_pDealerButton->scenePos() - calcDealerBtnPos(snap->s_dealer)) / steps;
		
		for (int i = 0; i < static_cast<int>(steps); ++i)
			anim->setPosAt(
				i / steps,
				QPointF(m_pDealerButton->scenePos() - (ptStep * i)));

		timer->start();
	}
	
	// Pots
	lblPots->setText(
		QString("Pot %1: %2").arg(
			snap->pots.size()).arg(
				std::accumulate(snap->pots.begin(), snap->pots.end(), 0.0), 0, 'f', 2));
	
	// CommunityCards
	if (snap->state == Table::NewRound)
	{
		for (unsigned int i = 0; i < m_CommunityCards.size(); i++)
		{
			this->scene()->removeItem(m_CommunityCards[i]);
			delete m_CommunityCards[i];
		}

		m_CommunityCards.clear();
	}

	std::vector<Card> allcards;
	snap->communitycards.copyCards(&allcards);

	// load new community cards
	if(allcards.size() > m_CommunityCards.size())
	{
		for (unsigned int i = m_CommunityCards.size(); i < allcards.size(); i++)
		{
			QGraphicsPixmapItem *p = this->scene()->addPixmap(
				QPixmap(
					QString("gfx/deck/default/%1.png").arg(allcards[i].getName())));

			p->setPos(calcCCardsPos(i));

			m_CommunityCards.push_back(p);
		}
	}

//	this->viewport()->update();	// TODO: remove
	
	if (snap->my_seat != -1)
	{
		seatinfo *s = &(snap->seats[snap->my_seat]);
		
		// minimum bet
		if (snap->minimum_bet > s->stake)
			m_pSliderAmount->setMinimum((int) s->stake);
		else
			m_pSliderAmount->setMinimum((int) snap->minimum_bet);
		
		// maximum bet is stake size
		m_pSliderAmount->setMaximum((int) s->stake);
		
		// show correct actions
		if (!s->in_round ||
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
		
		
		// determine hand-strength
		if (s->in_round)
		{
			vector<Card> allcards;
			tinfo->holecards.copyCards(&allcards);
			
			if (allcards.size())
			{
				HandStrength strength;
				GameLogic::getStrength(&(tinfo->holecards), &(snap->communitycards), &strength);
				const char *sstrength = HandStrength::getRankingName(strength.getRanking());
				
				lblHandStrength->setText(sstrength);
			}
			else
				lblHandStrength->clear();
		}
		else
			lblHandStrength->clear();
	}
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
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	Q_ASSERT_X(tinfo, __func__, "getTableInfo failed");
	
	table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, __func__, "invalid snapshot pointer");

	if (snap->my_seat == -1)
		return;

	const qreal amount = static_cast<qreal>(m_pSliderAmount->value());
	
	if (amount == snap->seats[snap->my_seat].stake + snap->seats[snap->my_seat].bet)
		((PClient*)qApp)->doSetAction(m_nGid, Player::Allin);
	else
		((PClient*)qApp)->doSetAction(m_nGid, Player::Raise, amount);
}

void WTable::actionShow()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Show);
}

void WTable::actionMuck()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Muck);
}

void WTable::slotShow()
{
	updateView();
	show();
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

		for(unsigned int i = 0; i < m_CommunityCards.size(); ++i)
		{
			m_CommunityCards[i]->setPos(
				m_CommunityCards[i]->pos().x() * ratio_x,
				m_CommunityCards[i]->pos().y() * ratio_y);
				
			m_CommunityCards[i]->scale(ratio_x, ratio_y);				
		}
	}	

	m_pChat->move(20, size.height() - m_pChat->height() - 10);

	m_LayoutActions->move(
		m_pChat->width() + ((size.width() - m_pChat->width() - m_LayoutActions->width()) / 2),
		size.height() - (m_pChat->height() / 2) - (m_LayoutActions->height() / 2));

	lblPots->move(
		(size.width() - lblPots->width()) / 2,
		200);
	
	lblHandStrength->move(
		(size.width() - lblHandStrength->width()) / 2,
		220);
	
	if (!config.getBool("ui_show_handstrength"))
		lblHandStrength->setVisible(false);
}
