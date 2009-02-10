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

const unsigned int WTable::nMaxSeats = 10;

WTable::WTable(int gid, int tid, QWidget *parent)
:	QGraphicsView(parent),
	m_nGid(gid),
	m_nTid(tid),
	m_pImgTable(0)
{
	//setAttribute(Qt::WA_DeleteOnClose); // FIXME: implement correctly

	// scene
	QGraphicsScene* pScene = new QGraphicsScene(0, 0, 900, 700, this);

	pScene->setBackgroundBrush(QPixmap("gfx/table/background.png"));
	
	// don't use bsptree
	pScene->setItemIndexMethod(QGraphicsScene::NoIndex);

	m_pImgTable = new QGraphicsPixmapItem(QPixmap("gfx/table/table.png"));
	m_pImgTable->setTransformationMode(Qt::SmoothTransformation);
	m_pImgTable->scale(
		pScene->width() / m_pImgTable->pixmap().width(),
		(pScene->height() - 150) / m_pImgTable->pixmap().height()); // 150 == height chatbox

	pScene->addItem(m_pImgTable);

	m_pDealerButton = new DealerButton;
	// QGraphicsPixmapItem(QPixmap("gfx/table/dealer_button.png"));
	m_pDealerButton->scale(0.5, 0.5);
	m_pDealerButton->setPos(
		pScene->width() / 5, pScene->height() / 2);

	pScene->addItem(m_pDealerButton);
/* TODO: fix
	m_pImgTimeout = new QGraphicsPixmapItem(QPixmap("gfx/table/timeout.png"));
	m_pImgTimeout->setTransformationMode(Qt::SmoothTransformation);
	m_pImgTimeout->scale(0.5, 0.5);
	m_pImgTimeout->setPos(50, 50);

	pScene->addItem(m_pImgTimeout);
	
	m_timeLine.setDuration(5000);
	m_timeLine.setFrameRange(0, m_pImgTimeout->pixmap().width());
	
	m_animTimeout.setItem(m_pImgTimeout);
	m_animTimeout.setTimeLine(&m_timeLine);

	for (int i = 0; i < 5000; ++i)
		m_animTimeout.setShearAt(
			i / 5000,
			m_timeLine.frameForTime(5000 - i),
			0);
*/
	for (unsigned int j = 0; j < 5; j++)
	{
		m_CommunityCards[j] = new QGraphicsPixmapItem(
			QPixmap(QString("gfx/deck/default/back.png")));

		m_CommunityCards[j]->setPos(
			calcCCardsPos(j, static_cast<int>(pScene->width())));
		m_CommunityCards[j]->setTransformationMode(Qt::SmoothTransformation);
		m_CommunityCards[j]->scale(0.3, 0.3);
		m_CommunityCards[j]->setZValue(5.0);

		pScene->addItem(m_CommunityCards[j]);
	}

	for (unsigned int i = 0; i < nMaxSeats; i++)
	{
		wseats[i] = new Seat(i, this);
		wseats[i]->scale(0.5, 0.5);
		wseats[i]->setPos(calcSeatPos(i));

		pScene->addItem(wseats[i]);
	}

	// view
	this->setScene(pScene);
//	this->setRenderHint(QPainter::HighQualityAntialiasing);
	this->setRenderHint(QPainter::SmoothPixmapTransform);
	this->setCacheMode(QGraphicsView::CacheBackground);
	
	// TODO: kannst du die min-window size noch runtersetzen? kann so auf meinem EeePC in der 
	// schule nicht das ganze fenster sehen :)
	this->setMinimumSize(
		static_cast<int>(pScene->width()),
		static_cast<int>(pScene->height()));
	this->setWindowTitle(tr("HoldingNuts table"));
	this->setWindowIcon(QIcon(":/res/pclient.ico"));
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
	
	QPushButton *btnBack = new QPushButton(tr("I'm back"), this);
	connect(btnBack, SIGNAL(clicked()), this, SLOT(actionBack()));
	
	QPushButton *btnSitout = new QPushButton(tr("Sitout"), this);
	connect(btnSitout, SIGNAL(clicked()), this, SLOT(actionSitout()));
	
	chkAutoFoldCheck = new QCheckBox(tr("Fold/Check"), this);
	chkAutoCheckCall = new QCheckBox(tr("Check/Call"), this);
	
	m_pSliderAmount = new EditableSlider;

	QHBoxLayout *lActions = new QHBoxLayout();
	lActions->addWidget(btnFold);
	lActions->addWidget(btnCheckCall);
	lActions->addWidget(btnBetRaise);
	lActions->addWidget(m_pSliderAmount);
	
	QHBoxLayout *lPreActions = new QHBoxLayout();
	lPreActions->addWidget(chkAutoFoldCheck);
	lPreActions->addWidget(chkAutoCheckCall);
	lPreActions->addWidget(btnSitout);   // FIXME: display outside StackedLayout
	
	QHBoxLayout *lPostActions = new QHBoxLayout();
	lPostActions->addWidget(btnMuck);
	lPostActions->addWidget(btnShow);
	
	QHBoxLayout *lSitoutActions = new QHBoxLayout();
	lSitoutActions->addWidget(btnBack);
	
	stlayActions = new QStackedLayout();
	QWidget *pageActions = new QWidget(this);
	QWidget *pagePreActions = new QWidget(this);
	QWidget *pagePostActions = new QWidget(this);
	QWidget *pageNoActions = new QWidget(this);
	QWidget *pageSitoutActions = new QWidget(this);
	
	pageActions->setLayout(lActions);
	pagePreActions->setLayout(lPreActions);
	pagePostActions->setLayout(lPostActions);
	pageSitoutActions->setLayout(lSitoutActions);
	
	m_nActions = stlayActions->addWidget(pageActions);
	m_nPreActions = stlayActions->addWidget(pagePreActions);
	m_nPostActions = stlayActions->addWidget(pagePostActions);
	m_nNoAction = stlayActions->addWidget(pageNoActions);
	m_nSitoutActions = stlayActions->addWidget(pageSitoutActions);

	m_LayoutActions = new QLabel(this);
	m_LayoutActions->setPixmap(QPixmap("gfx/table/actions.png"));
	m_LayoutActions->setScaledContents(true);
	m_LayoutActions->setFixedSize(400, 70);
	m_LayoutActions->setLayout(stlayActions);
	
	m_pChat	= new ChatBox("", m_nGid, m_nTid, ChatBox::INPUTLINE_BOTTOM, 120, this);
	m_pChat->show();
	
	lblPots = new QLabel("Pot 0: 0.00", this);
	lblPots->setFixedWidth(static_cast<int>(pScene->width()));
	lblPots->setAlignment(Qt::AlignCenter);
	
	lblHandStrength = new QLabel("HandStrength", this);
	lblHandStrength->setFixedWidth(static_cast<int>(pScene->width()));
	lblHandStrength->setAlignment(Qt::AlignCenter);
	
	if (!config.getBool("ui_show_handstrength"))
		lblHandStrength->setVisible(false);
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

QPointF WTable::calcCCardsPos(unsigned int nCard, int table_width) const
{
	Q_ASSERT_X(nCard < 5, __func__, "invalided Card Number");

	static const qreal card_spacing = 8;
	static const qreal card_width = 80;	// card_width = 72 + spacing

	return QPointF(
		((table_width - (5 * card_width - card_spacing)) / 2) + nCard * card_width,
		275);
}

void WTable::updateView()
{
	int my_cid = ((PClient*)qApp)->getMyCId();
	
	const gameinfo *ginfo = ((PClient*)qApp)->getGameInfo(m_nGid);
	Q_ASSERT_X(ginfo, __func__, "getGameInfo failed");
	
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	Q_ASSERT_X(tinfo, __func__, "getTableInfo failed");

	const table_snapshot *snap = &(tinfo->snap);
	Q_ASSERT_X(snap, __func__, "invalid snapshot pointer");
	
	
	for (unsigned int i=0; i < nMaxSeats; i++)
	{
		const seatinfo *seat = &(snap->seats[i]);
		
		if (seat->valid)
		{
			int cid = seat->client_id;
			playerinfo *pinfo = ((PClient*)qApp)->getPlayerInfo(cid);
			
			if (pinfo)
				wseats[i]->setName(pinfo->name);
			else
				wseats[i]->setName("???");
			
			wseats[i]->setStake(seat->stake);
			wseats[i]->setValid(true);
			wseats[i]->setSitout(seat->sitout);
			
			if (snap->state > Table::ElectDealer)
			{
				wseats[i]->setCurrent(snap->s_cur == i, ginfo->player_timeout);
				// m_timeLine.start();
			}
			
			if (seat->in_round)
			{
				if (seat->stake == 0)
					wseats[i]->setAction(Player::Allin, seat->bet);
				else
					wseats[i]->setAction(seat->action, seat->bet);
				
				wseats[i]->setMySeat(my_cid == cid);
				
				std::vector<Card> allcards;
				
				if (my_cid == cid)
					tinfo->holecards.copyCards(&allcards);
				else
					seat->holecards.copyCards(&allcards);
				
				if (allcards.size())
				{
					char card1[3], card2[3];
					strcpy(card1, allcards[0].getName());
					strcpy(card2, allcards[1].getName());
					wseats[i]->setCards(card1, card2);
					
					wseats[i]->showBigCards(true);
				}
				else
				{
					if (my_cid == cid)
						wseats[i]->setCards("blank", "blank");
					else
						wseats[i]->setCards("back", "back");
					
					wseats[i]->showBigCards(false);
				}
				
				wseats[i]->showSmallCards(true);
			}
			else   // player isn't anymore involved in current hand
			{
				wseats[i]->setAction(seat->action);
				wseats[i]->setCards("blank", "blank");
				wseats[i]->showBigCards(false);
				wseats[i]->showSmallCards(false);
			}

//			this->scene()->update(wseats[i]->boundingRect());
			
		}
		else
			wseats[i]->setValid(false);
	}
	
	// dealerbutton
	if (snap->state == Table::NewRound)
	{
		m_pDealerButton->startAnimation(
			wseats[snap->s_dealer]->pos() + wseats[snap->s_dealer]->boundingRectSeat().center());
	}
	
	// Pots
	QString strPots;
	for (unsigned int t = 0; t < snap->pots.size(); ++t)
	{
		strPots.append(
			QString("Pot %1: %2 ").arg(t+1).arg(snap->pots.at(t), 0, 'f', 2));
	}
	lblPots->setText(strPots);
	
	// CommunityCards
	if (snap->state == Table::NewRound)
		for (unsigned int i = 0; i < 5; i++)
			m_CommunityCards[i]->hide();

	std::vector<Card> allcards;
	snap->communitycards.copyCards(&allcards);
	
	// load community cards
	for (unsigned int i = 0; i < allcards.size(); i++)
	{
		m_CommunityCards[i]->setPixmap(
			QPixmap(
				QString("gfx/deck/default/%1.png").arg(allcards[i].getName())));
		m_CommunityCards[i]->show();
	}

	this->viewport()->update();	// TODO: remove
	
	if (snap->my_seat != -1)
	{
		const seatinfo *s = &(snap->seats[snap->my_seat]);
		
		// minimum bet
		if (snap->minimum_bet > s->stake)
			m_pSliderAmount->setMinimum((int) s->stake);
		else
			m_pSliderAmount->setMinimum((int) snap->minimum_bet);
		
		// maximum bet is stake size
		m_pSliderAmount->setMaximum((int) s->stake);
		
		// show correct actions
		if (s->sitout)
		{
			stlayActions->setCurrentIndex(m_nSitoutActions);
		}
		else if (!s->in_round ||
			snap->state == Table::AllFolded ||
			snap->state == Table::Showdown ||
			snap->state == Table::EndRound)
		{
			stlayActions->setCurrentIndex(m_nNoAction);
		}
		else
		{
			if (snap->state == Table::AskShow)
			{
				if ((int)snap->s_cur == snap->my_seat)
					stlayActions->setCurrentIndex(m_nPostActions);
				else
					stlayActions->setCurrentIndex(m_nNoAction);
			}
			else
			{
				if ((int)snap->s_cur == snap->my_seat)
				{
					if (chkAutoFoldCheck->checkState() == Qt::Checked)
					{
						if (greaterBet(*snap, s->bet))
							actionFold();
						else
							actionCheckCall();
						
						chkAutoFoldCheck->setCheckState(Qt::Unchecked);
					}
					else if (chkAutoCheckCall->checkState() == Qt::Checked)
					{
						actionCheckCall();
						
						chkAutoCheckCall->setCheckState(Qt::Unchecked);
					}
					else
					{
 						if ((int)s->stake == 0) // TODO: snap->state == Table::Showdown???? see line 584
							stlayActions->setCurrentIndex(m_nNoAction);
						else
							stlayActions->setCurrentIndex(m_nActions);
					}
				}
				else
				{
					if (s->stake > 0)
					{
						stlayActions->setCurrentIndex(m_nPreActions);
						
						qreal greater_bet = 0;
// qDebug() << "id= " << snap->s_cur << " round= " << Table::BettingRound(snap->betting_round)<< " s->bet= " << s->bet;
						// validate auto-call
						if (greaterBet(*snap, s->bet, &greater_bet))
						{
							chkAutoCheckCall->setText(QString("Call %1").arg(greater_bet - s->bet));
							chkAutoCheckCall->setCheckState(Qt::Unchecked);
						}
					}
				}
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
		((PClient*)qApp)->doSetAction(m_nGid, Player::Raise, amount + snap->seats[snap->my_seat].bet);
}

void WTable::actionShow()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Show);
}

void WTable::actionMuck()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Muck);
}

void WTable::doSitout(bool bSitout)
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;
	
	const table_snapshot *snap = &(tinfo->snap);
	if (!snap)
		return;
	
	if (snap->my_seat == -1)
		return;
	
	seatinfo *seat = (seatinfo*) &(snap->seats[snap->my_seat]);
	
	// set sitout and show available actions again
	seat->sitout = bSitout;
	updateView();
}

void WTable::actionSitout()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Sitout);
	
	doSitout(true);
}

void WTable::actionBack()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Back);
	
	doSitout(false);
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

		for (unsigned int i = 0; i < 5; ++i)
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
}

bool WTable::greaterBet(const table_snapshot& snap, const qreal& bet, qreal *pbet) const
{
	for (unsigned int i=0; i < nMaxSeats; i++)
	{
		const seatinfo *seat = &(snap.seats[i]);

		if (seat->valid && seat->bet > bet)
		{
			if (pbet)
				*pbet = seat->bet;

			return true;
		}
	}

	return false;
}
