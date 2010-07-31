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
 *     Michael Miller <michael.miller@holdingnuts.net>
 */


#include <cstdio>
#include <cmath>
#include <numeric>

#include "Config.h"
#include "Debug.h"
#include "Logger.h"
#include "ConfigParser.hpp"
#include "SysAccess.h"
#include "GameLogic.hpp"

#include "pclient.hpp"

#include "WTable.hpp"
#include "ChatBox.hpp"
#include "Seat.hpp"
#include "DealerButton.hpp"
#include "EditableSlider.hpp"
#include "TimeOut.hpp"
#include "PlayerListTableModel.hpp"
#include "ChipStack.hpp"

#ifndef NOAUDIO
# include "Audio.h"
#endif
#include "data.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOption>
#include <QTime>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include <QShortcut>
#include <QMenu>
#include <QPushButton>
#include <QCheckBox>
#include <QCompleter>
#include <QListView>

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
			s << "unknown";
	}
	return s;
}

QDebug operator << (QDebug s, const seatinfo& si)	
{
	s << "seatinfo" << "\n";
	s << "\t client_id= " << si.client_id << "\n";
	s << "\t valid= " << (si.valid ? "true" : "false") << "\n";
	s << "\t in_round= " << (si.in_round ? "true" : "false") << "\n";
	s << "\t sitout= " << (si.sitout ? "true" : "false") << "\n";
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

	s << "\t pots= ";
	for(unsigned i = 0; i < t.pots.size(); ++i)
		s << "[" << i << "]= " << t.pots.at(i);
	s << "\n";

	s << "\t my_seat= " << t.my_seat << "\n";
	s << "end table_snapshot" << "\n";
	
	return s;
}

#endif /* DEBUG */

const unsigned int WTable::nMaxSeats = 10;

WTable::WTable(int gid, int tid, QWidget *parent)
:	QWidget(parent),
	m_nGid(gid),
	m_nTid(tid),
	m_pImgTable(0),
	sizeCommunityCards(120, 170),
	posYCommunityCards(360),
	posYTxtPots(290),
	posYPots(270)
{
	//setAttribute(Qt::WA_DeleteOnClose); // FIXME: implement correctly

	// scene
	m_pScene = new QGraphicsScene(this);

	//m_pScene->setBackgroundBrush(Qt::black);//QPixmap("gfx/table/background.png"));
	// don't use bsptree
	m_pScene->setItemIndexMethod(QGraphicsScene::NoIndex);

	m_pImgTable = new QGraphicsPixmapItem(QPixmap("gfx/table/table.png"));
	m_pImgTable->setTransformationMode(Qt::SmoothTransformation);

	m_pScene->addItem(m_pImgTable);

	m_pDealerButton = new DealerButton;
	m_pDealerButton->setPos(m_pImgTable->boundingRect().center());
	
	m_pScene->addItem(m_pDealerButton);

	m_pTimeout = new TimeOut;
	m_pTimeout->hide();

	connect(m_pTimeout, SIGNAL(timeup(int)), this, SLOT(slotTimeup(int)));
	connect(m_pTimeout, SIGNAL(quarterElapsed(int)), this, SLOT(slotFirstReminder(int)));
	connect(m_pTimeout, SIGNAL(threeQuarterElapsed(int)), this, SLOT(slotSecondReminder(int)));

	m_pScene->addItem(m_pTimeout);

	for (unsigned int j = 0; j < 5; j++)
	{
		m_CommunityCards[j] = new QGraphicsPixmapItem(
				QPixmap(QString("gfx/deck/%1/blank.png")
				.arg(QString::fromStdString(config.get("ui_card_deck")))));

		const QRectF& rc = m_CommunityCards[j]->boundingRect();

		m_CommunityCards[j]->setTransformationMode(Qt::SmoothTransformation);
		// fixed size for any kind of cardsets
		m_CommunityCards[j]->scale(
				sizeCommunityCards.width() / rc.width(),
				sizeCommunityCards.height() / rc.height());
		m_CommunityCards[j]->setZValue(5.0);
		m_CommunityCards[j]->setPos(calcCCardsPos(j));
		m_CommunityCards[j]->hide();

		m_pScene->addItem(m_CommunityCards[j]);
	}

	for (unsigned int i = 0; i < nMaxSeats; i++)
	{
		wseats[i] = new Seat(i, this);
		wseats[i]->setPos(calcSeatPos(i));

		m_pScene->addItem(wseats[i]);
		
		// calculate dealer button pos
		m_ptDealerBtn[i] = calcDealerBtnPos(i);
	}

	Seat::setInSeatFont(QFont("Arial", 18,  QFont::Bold));

	QFont font = QApplication::font();
	
	font.setPointSize(20); 
	font.setBold(true);
	
	const QFontMetrics fm(font);
	const QPointF ptCenter = m_pImgTable->boundingRect().center();
	
	m_pTxtPots = m_pScene->addSimpleText("Main pot: 0", font);
	m_pTxtPots->setPos(calcTxtPotsPos());
	m_pTxtPots->setZValue(3);
	
	font.setBold(false);
	
	m_pTxtHandStrength = m_pScene->addSimpleText("HandStrength", font);
	m_pTxtHandStrength->setPos(calcHandStrengthPos());
	m_pTxtHandStrength->setZValue(3);
	m_pTxtHandStrength->setVisible(config.getBool("ui_show_handstrength"));

	for (unsigned int t = 0; t < sizeof(m_Pots) / sizeof(m_Pots[0]); t++)
	{
		m_Pots[t] = new ChipStack;
		m_Pots[t]->setZValue(6);

		m_pScene->addItem(m_Pots[t]);
	}

	// view
	m_pView = new QGraphicsView(m_pScene);
	m_pView->setRenderHint(QPainter::SmoothPixmapTransform);
	m_pView->setCacheMode(QGraphicsView::CacheNone);
	m_pView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	m_pView->setOptimizationFlag(QGraphicsView::DontClipPainter, true);
	m_pView->setFrameStyle(QFrame::Plain);
	m_pView->setStyleSheet("background: transparent");
	
	
	// fixed width for action buttons which have an amount in their caption
	const unsigned int actionbtn_width = 100;
	
	// ui - widgets
	QPushButton *btnFold = new QPushButton(tr("&Fold"), this);
	btnFold->setFixedWidth(actionbtn_width);
	connect(btnFold, SIGNAL(clicked()), this, SLOT(actionFold()));
	
	btnCheckCall = new QPushButton("Check/Call", this);
	btnCheckCall->setFixedWidth(actionbtn_width);
	connect(btnCheckCall, SIGNAL(clicked()), this, SLOT(actionCheckCall()));
	
	btnBetRaise = new QPushButton("Bet/Raise", this);
	btnBetRaise->setFixedWidth(actionbtn_width);
	connect(btnBetRaise, SIGNAL(clicked()), this, SLOT(actionBetRaise()));
	
	QPushButton *btnShow = new QPushButton(tr("&Show"), this);
	connect(btnShow, SIGNAL(clicked()), this, SLOT(actionShow()));
	
	QPushButton *btnMuck = new QPushButton(tr("&Muck"), this);
	connect(btnMuck, SIGNAL(clicked()), this, SLOT(actionMuck()));
	
	
	/* Persistent actions on the right, e.g. Sitout/Back, "Muck losing hands" */
	QPushButton *btnBack = new QPushButton(tr("I'm bac&k"), this);
	btnBack->setFixedSize(actionbtn_width, 25);
	connect(btnBack, SIGNAL(clicked()), this, SLOT(actionBack()));
	
	QPushButton *btnSitout = new QPushButton(tr("Sit&out"), this);
	btnSitout->setFixedSize(actionbtn_width, 25);
	connect(btnSitout, SIGNAL(clicked()), this, SLOT(actionSitout()));
	
	stlayPersistentActions = new QStackedLayout();
	m_nSitout = stlayPersistentActions->addWidget(btnSitout);
	m_nBack = stlayPersistentActions->addWidget(btnBack);
	
	QCheckBox *chkAutoMuck = new QCheckBox(tr("Muck losing\nhands"), this);
	chkAutoMuck->setFont(QFont("Arial", 8));
	chkAutoMuck->setEnabled(false);	// FIXME: implement me...
	
	QVBoxLayout *layPersistentActions = new QVBoxLayout();
	layPersistentActions->addLayout(stlayPersistentActions);
	layPersistentActions->addWidget(chkAutoMuck, 0, Qt::AlignCenter);
	
	lblPersistentActions = new QLabel(this);
	lblPersistentActions->setPixmap(QPixmap("gfx/table/actions.png"));
	lblPersistentActions->setScaledContents(true);
	lblPersistentActions->setFixedSize(120, 90);
	lblPersistentActions->setLayout(layPersistentActions);
	
	
	
	// bet shortcuts
	const unsigned int raisebtn_width = 39;
	const unsigned int raisebtn_height = 18;

	btnBetsizeMinimum = new QPushButton(tr("Min"), this);
	btnBetsizeMinimum->setFixedSize(raisebtn_width, raisebtn_height);
	connect(btnBetsizeMinimum, SIGNAL(clicked()), this, SLOT(actionBetsizeMinimum()));
	
	btnBetsizeQuarterPot = new QPushButton(tr("1/4"), this);
	btnBetsizeQuarterPot->setFixedSize(raisebtn_width, raisebtn_height);
	connect(btnBetsizeQuarterPot, SIGNAL(clicked()), this, SLOT(actionBetsizeQuarterPot()));

	btnBetsizeHalfPot = new QPushButton(tr("1/2"), this);
	btnBetsizeHalfPot->setFixedSize(raisebtn_width, raisebtn_height);
	connect(btnBetsizeHalfPot, SIGNAL(clicked()), this, SLOT(actionBetsizeHalfPot()));

	btnBetsizeThreeQuarterPot = new QPushButton(tr("3/4"), this);
	btnBetsizeThreeQuarterPot->setFixedSize(raisebtn_width, raisebtn_height);
	connect(btnBetsizeThreeQuarterPot, SIGNAL(clicked()), this, SLOT(actionBetsizeThreeQuarterPot()));

	btnBetsizePotsize = new QPushButton(tr("Pot"), this);
	btnBetsizePotsize->setFixedSize(raisebtn_width, raisebtn_height);
	connect(btnBetsizePotsize, SIGNAL(clicked()), this, SLOT(actionBetsizePotsize()));

	btnBetsizeMaximum = new QPushButton(tr("Max"), this);
	btnBetsizeMaximum->setFixedSize(raisebtn_width, raisebtn_height);
	connect(btnBetsizeMaximum, SIGNAL(clicked()), this, SLOT(actionBetsizeMaximum()));

	QHBoxLayout *lPots = new QHBoxLayout();
	lPots->addStretch(2); 
	lPots->setContentsMargins(0, 0, 0, 0);
	lPots->addWidget(btnBetsizeMinimum, Qt::AlignRight);
	lPots->addWidget(btnBetsizeQuarterPot, Qt::AlignRight);
	lPots->addWidget(btnBetsizeHalfPot, Qt::AlignRight);
	lPots->addWidget(btnBetsizeThreeQuarterPot, Qt::AlignRight);
	lPots->addWidget(btnBetsizePotsize, Qt::AlignRight);
	lPots->addWidget(btnBetsizeMaximum, Qt::AlignRight);
	
	wRaiseBtns = new QWidget(this);
	wRaiseBtns->setLayout(lPots);
	
	
	chkAutoFoldCheck = new QCheckBox("Fold/Check", this);
	connect(chkAutoFoldCheck, SIGNAL(stateChanged(int)), this, SLOT(actionAutoFoldCheck(int)));
	chkAutoCheckCall = new QCheckBox("Check/Call", this);
	connect(chkAutoCheckCall, SIGNAL(stateChanged(int)), this, SLOT(actionAutoCheckCall(int)));
	
	m_pSliderAmount = new EditableSlider(this);
	connect(m_pSliderAmount, SIGNAL(dataChanged()), this, SLOT(slotBetRaiseAmountChanged()));
	connect(m_pSliderAmount, SIGNAL(returnPressed()), this, SLOT(actionBetRaise()));

	QHBoxLayout *lActionsBtns = new QHBoxLayout();
	lActionsBtns->addWidget(btnFold);
	lActionsBtns->addWidget(btnCheckCall);
	lActionsBtns->addWidget(btnBetRaise);
	lActionsBtns->addWidget(m_pSliderAmount);
	
	
	QVBoxLayout *lActions = new QVBoxLayout();	
	lActions->addWidget(wRaiseBtns);
	lActions->addLayout(lActionsBtns);
	
	QHBoxLayout *lPreActionsAuto = new QHBoxLayout();
	lPreActionsAuto->addWidget(chkAutoFoldCheck);
	lPreActionsAuto->addWidget(chkAutoCheckCall);
	
	QVBoxLayout *lPreActions = new QVBoxLayout();
	lPreActions->addLayout(lPreActionsAuto);
	
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
	
	m_nActions = stlayActions->addWidget(pageActions);
	m_nPreActions = stlayActions->addWidget(pagePreActions);
	m_nPostActions = stlayActions->addWidget(pagePostActions);
	m_nNoAction = stlayActions->addWidget(pageNoActions);
	
	QLabel *lblActions = new QLabel(this);
	lblActions->setPixmap(QPixmap("gfx/table/actions.png"));
	lblActions->setScaledContents(true);
	lblActions->setFixedSize(450, 90);
	lblActions->setLayout(stlayActions);
	
#if 0
	QListView *popupView = new QListView;
	popupView->setModelColumn(1);
	
	QCompleter *cmpChat = new QCompleter(((PClient*)qApp)->playerList(), this);
	cmpChat->setPopup(popupView);
	cmpChat->setCompletionRole(Qt::DisplayRole);
	cmpChat->setCompletionColumn(((PClient*)qApp)->playerList()->nameColumn());
#endif

	m_pChat	= new ChatBox(ChatBox::INPUTLINE_BOTTOM, 0, this);
	m_pChat->setFixedHeight(150);
	m_pChat->setFontPointSize(m_pChat->fontPointSize() - 1);
	//m_pChat->setCompleter(cmpChat);
	connect(m_pChat, SIGNAL(dispatchedMessage(QString)), this, SLOT(actionChat(QString)));

	QGridLayout *mainLayout = new QGridLayout(this);

	mainLayout->addWidget(m_pView, 0, 0, 1, 7);
	mainLayout->addWidget(m_pChat, 1, 0);
	mainLayout->addWidget(lblActions, 1, 3);
	mainLayout->addWidget(lblPersistentActions, 1, 5);
	mainLayout->setColumnStretch(2, 2);
	mainLayout->setColumnStretch(4, 2);

	this->setLayout(mainLayout);
	
	// shortcuts
	shortcutFold = new QShortcut(tr("Ctrl+F"), btnFold);
	connect(shortcutFold, SIGNAL(activated()), this, SLOT(actionFold()));

	shortcutCallCheck = new QShortcut(tr("Ctrl+C"), btnCheckCall);
	connect(shortcutCallCheck, SIGNAL(activated()), this, SLOT(actionCheckCall()));

	shortcutBet = new QShortcut(tr("Ctrl+B"), btnBetRaise);
	connect(shortcutBet, SIGNAL(activated()), this, SLOT(actionBetRaise()));

	shortcutRaise = new QShortcut(tr("Ctrl+R"), btnBetRaise);
	connect(shortcutRaise, SIGNAL(activated()), this, SLOT(actionBetRaise()));

	shortcutAllin = new QShortcut(tr("Ctrl+A"), btnBetRaise);
	shortcutAllin->setEnabled(false);
	connect(shortcutAllin, SIGNAL(activated()), this, SLOT(actionBetRaise()));

	shortcutMuck = new QShortcut(tr("Ctrl+M"), btnMuck);
	connect(shortcutMuck, SIGNAL(activated()), this, SLOT(actionMuck()));

	shortcutShow = new QShortcut(tr("Ctrl+S"), btnShow);
	connect(shortcutShow, SIGNAL(activated()), this, SLOT(actionShow()));
	
	shortcutSitout = new QShortcut(tr("Ctrl+O"), this);
	connect(shortcutSitout, SIGNAL(activated()), this, SLOT(actionSitout()));
	
	shortcutBack = new QShortcut(tr("Ctrl+K"), this);
	connect(shortcutBack, SIGNAL(activated()), this, SLOT(actionBack()));
	
	// assign shortcut for making screenshot
	QShortcut *shortcutScreenshot = new QShortcut(Qt::Key_F10, this);
	connect(shortcutScreenshot, SIGNAL(activated()), this, SLOT(actionScreenshot()));
	
	// set background
	QPalette p(this->palette());
	p.setBrush(QPalette::Window, QBrush(QPixmap("gfx/table/background.png")));
	this->setPalette(p);	
	
	this->setMinimumSize(640, 480);
	this->setWindowIcon(QIcon(":/res/hn_logo.png"));
	
	// load gui settings
	QSettings settings;

	settings.beginGroup("TableWindow");
		this->resize(settings.value("size", QSize(800, 630)).toSize());
		this->move(settings.value("pos", QPoint(50, 50)).toPoint());
	settings.endGroup();
}

WTable::~WTable()
{
	delete m_pImgTable;
	delete m_pDealerButton;
	delete m_pTimeout;
	delete m_pTxtPots;
	delete m_pTxtHandStrength;

	for (unsigned int j = 0; j < 5; j++)
		delete m_CommunityCards[j];

	for (unsigned int i = 0; i < nMaxSeats; i++)
		delete wseats[i];
		
	delete stlayActions;
	delete stlayPersistentActions;
}

QPointF WTable::calcSeatPos(unsigned int nSeatID) const
{
	Q_ASSERT_X(nSeatID < nMaxSeats, Q_FUNC_INFO, "invalid Seat Number");
	Q_ASSERT_X(wseats[nSeatID], Q_FUNC_INFO, "bad seat pointer");
	Q_ASSERT_X(m_pImgTable, Q_FUNC_INFO, "bad table image pointer");

	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3
	
	const QRectF& rcSeat = wseats[nSeatID]->boundingRectSeat();
	const QRectF& rcTable = this->m_pImgTable->boundingRect();

	switch (nSeatID)
	{
		case 0:
			return QPointF(rcTable.width() * 0.65, 0);
		case 1:
			return QPointF(rcTable.width() - rcSeat.width() * 0.75, rcTable.height() * 0.30);
		case 2:
			return QPointF(rcTable.width() - rcSeat.width() * 0.75, rcTable.height() * 0.65);
		case 3:
			return QPointF(rcTable.width() * 0.65, rcTable.height() - rcSeat.height());
		case 4:
			return QPointF(rcTable.width() * 0.5 - rcSeat.width() * 0.5, rcTable.height() - rcSeat.height());
		case 5:
			return QPointF(rcTable.width() * 0.35 - rcSeat.width(), rcTable.height() - rcSeat.height());
		case 6:
			return QPointF(-(rcSeat.width() * 0.25), rcTable.height() * 0.65);
		case 7:
			return QPointF(-(rcSeat.width() * 0.25), rcTable.height() * 0.30);
		case 8:
			return QPointF(rcTable.width() * 0.35 - rcSeat.width(), 0);
		case 9:
			return QPointF(rcTable.width() * 0.5 - rcSeat.width() * 0.5, 0);
	}
	return QPointF(0, 0);
}

QPointF WTable::calcCCardsPos(unsigned int nCard) const
{
	Q_ASSERT_X(nCard < 5, Q_FUNC_INFO, "invalid Card Number");
	Q_ASSERT_X(m_pScene, Q_FUNC_INFO, "bad scene pointer");
	Q_ASSERT_X(m_CommunityCards[nCard], Q_FUNC_INFO, "bad community card pointer");

	QRectF rc = m_CommunityCards[nCard]->boundingRect();
	const QTransform m = m_CommunityCards[nCard]->transform();
	
	rc = m.mapRect(rc);

	const qreal card_spacing = rc.width() * 0.1;
	const qreal card_width = rc.width() + card_spacing;

	return QPointF(
		((m_pScene->width() - (5 * card_width - card_spacing)) / 2) + nCard * card_width,
		posYCommunityCards);
}

QPointF WTable::calcTimeoutPos(unsigned int nSeatID) const
{
	Q_ASSERT_X(nSeatID < nMaxSeats, Q_FUNC_INFO, "invalid Seat Number");
	Q_ASSERT_X(wseats[nSeatID], Q_FUNC_INFO, "bad seat pointer");
	Q_ASSERT_X(m_pTimeout, Q_FUNC_INFO, "bad timeout pointer");
	
	QPointF pt = wseats[nSeatID]->scenePos();

	pt.ry() += wseats[nSeatID]->boundingRectSeat().height() - 
		m_pTimeout->boundingRect().height();

	return pt;
}

QPointF WTable::calcHandStrengthPos() const
{
	Q_ASSERT_X(m_pTxtHandStrength, Q_FUNC_INFO, "bad hand strength pointer");
	Q_ASSERT_X(m_pImgTable, Q_FUNC_INFO, "bad table image pointer");

	static const QFontMetrics fm(m_pTxtHandStrength->font());
	static const QPointF ptCenter = m_pImgTable->boundingRect().center();
	
	return QPointF(
		ptCenter.x() - (fm.width(m_pTxtHandStrength->text()) / 2),
		posYTxtPots + fm.height());
}

QPointF WTable::calcTxtPotsPos() const
{
	Q_ASSERT_X(m_pTxtPots, Q_FUNC_INFO, "bad text pots pointer");
	Q_ASSERT_X(m_pImgTable, Q_FUNC_INFO, "bad table image pointer");

	static const QFontMetrics fm(m_pTxtPots->font());
	static const QPointF ptCenter = m_pImgTable->boundingRect().center();
	
	return QPointF(
		ptCenter.x() - (fm.width(m_pTxtPots->text()) / 2),
		posYTxtPots);
}

QPointF WTable::calcDealerBtnPos(unsigned int nSeatID) const
{
	Q_ASSERT_X(nSeatID < nMaxSeats, Q_FUNC_INFO, "invalid Seat Number");
	Q_ASSERT_X(wseats[nSeatID], Q_FUNC_INFO, "bad seat pointer");
	Q_ASSERT_X(m_pDealerButton, Q_FUNC_INFO, "bad table image pointer");
	
	QPointF pt = wseats[nSeatID]->sceneBoundingRect().center();

	//		8	9	0
	//	 7			   1
	// 						
	//   6			   2
	//		5	4	3
	
	switch (nSeatID)
	{
		case 0: case 8: case 9:
				pt.ry() += (wseats[nSeatID]->sceneBoundingRect().height() * 0.5f + 20);
				pt.rx() += 10;
			break;
		case 1:
				pt.rx() -= (
					wseats[nSeatID]->sceneBoundingRect().width() * 0.5f + 
					m_pDealerButton->sceneBoundingRect().width() + 
					30);
				pt.ry() += 70;
			break;
		case 2:
				pt.rx() -= (
					wseats[nSeatID]->sceneBoundingRect().width() * 0.5f + 
					m_pDealerButton->sceneBoundingRect().width() + 
					30);
				pt.ry() -= 40;	
			break;
		case 3: case 4: case 5:
				pt.ry() -= (
					wseats[nSeatID]->sceneBoundingRect().height() * 0.5f + 
					m_pDealerButton->sceneBoundingRect().height() +
					10);
				pt.rx() -= 30;
			break;
		case 6:
				pt.rx() += wseats[nSeatID]->sceneBoundingRect().width() * 0.5f + 20;
			break;
		case 7:
				pt.rx() += wseats[nSeatID]->sceneBoundingRect().width() * 0.5f + 20;
				pt.ry() += 70;
			break;
	}

	return pt;
}

void WTable::calcPotsPos()
{
	static const qreal space = 5;

	qreal wholePotsWidth = 0;

	for (unsigned int t = 0; t < sizeof(m_Pots) / sizeof(m_Pots[0]); t++)
	{
		if (m_Pots[t]->isVisible())
			wholePotsWidth += m_Pots[t]->boundingRect().width() + space;
	}
	
	qreal xStart = m_pImgTable->boundingRect().center().x() - (wholePotsWidth * 0.5);
	
	// place the pots on the table
	for (unsigned int t = 0; t < sizeof(m_Pots) / sizeof(m_Pots[0]); t++)
	{
		if (m_Pots[t]->isVisible())
		{
			m_Pots[t]->setPos(xStart, posYPots);
		
			xStart += m_Pots[t]->boundingRect().width() + space;
		}
	}
}

void WTable::evaluateActions(const table_snapshot *snap)
{
	// player does not sit at table
	if (snap->my_seat == -1)
	{
		stlayActions->setCurrentIndex(m_nNoAction);
		lblPersistentActions->setEnabled(false);
		return;
	}
	else
		lblPersistentActions->setEnabled(true);
	
	
	const seatinfo *s = &(snap->seats[snap->my_seat]);
	
	// minimum bet
	if (snap->minimum_bet > s->stake)
	{
		m_pSliderAmount->setMinimum(s->stake + s->bet);
		m_pSliderAmount->setEnabled(false);
	}
	else
	{
		m_pSliderAmount->setMinimum(snap->minimum_bet);
		m_pSliderAmount->setEnabled(true);
	}
	
	// maximum bet is stake size
	m_pSliderAmount->setMaximum(s->stake + s->bet);
	
	
	// evaluate available actions
	if (s->sitout)
	{
		stlayActions->setCurrentIndex(m_nNoAction);
	}
	else if (!s->in_round ||
		!(snap->state == Table::Blinds ||
			snap->state == Table::Betting ||
			snap->state == Table::AskShow) ||
		(snap->state != Table::AskShow && snap->nomoreaction))
	{
		stlayActions->setCurrentIndex(m_nNoAction);
	}
	else if (snap->state == Table::AskShow)
	{
		if (snap->s_cur == snap->my_seat)
		{
			setForegroundWindow();
			stlayActions->setCurrentIndex(m_nPostActions);
		}
		else
			stlayActions->setCurrentIndex(m_nNoAction);
	}
	else
	{
		// re-enable if actions were disabled by something else
		btnCheckCall->setEnabled(true);
		btnBetRaise->setEnabled(true);
		
		
		chips_type greatest_bet = 0;
		bool bGreaterBet = greaterBet(snap, s->bet, &greatest_bet);
		
		if (snap->s_cur == snap->my_seat)
		{
			if (s->stake == 0)
				stlayActions->setCurrentIndex(m_nNoAction);
			else
			{
				setForegroundWindow();
				
				if (bGreaterBet)
				{
					btnCheckCall->setText(tr("&Call %1").arg(greatest_bet - s->bet));
					btnBetRaise->setText(tr("&Raise %1").arg(m_pSliderAmount->value()));
					
					shortcutBet->setEnabled(false);
					shortcutRaise->setEnabled(true);
				}
				else
				{
					btnCheckCall->setText(tr("&Check"));
					
					if (greaterBet(snap, 0))
					{
						btnBetRaise->setText(tr("&Raise %1").arg(m_pSliderAmount->value()));
						
						shortcutBet->setEnabled(false);
						shortcutRaise->setEnabled(true);
					}
					else
					{
						btnBetRaise->setText(tr("&Bet %1").arg(m_pSliderAmount->value()));
						
						shortcutBet->setEnabled(true);
						shortcutRaise->setEnabled(false);
					}
				}
				
				// handle allin-case
				if (greatest_bet >= s->stake + s->bet)
				{
					btnCheckCall->setVisible(false);
					m_pSliderAmount->setVisible(false);
					wRaiseBtns->setVisible(false);
					
					btnBetRaise->setText(tr("&Allin %1").arg(s->stake));
					shortcutAllin->setEnabled(true);
					shortcutBet->setEnabled(false);
					shortcutRaise->setEnabled(false);
				}
				else if (snap->minimum_bet >= s->stake + s->bet)
				{
					btnCheckCall->setVisible(true);
					m_pSliderAmount->setVisible(false);
					wRaiseBtns->setVisible(false);
					
					btnBetRaise->setText(tr("&Allin %1").arg(s->stake));
					shortcutAllin->setEnabled(true);
					shortcutBet->setEnabled(false);
					shortcutRaise->setEnabled(false);
				}
				else
				{
					btnCheckCall->setVisible(true);
					m_pSliderAmount->setVisible(true);
					wRaiseBtns->setVisible(true);
					
					shortcutAllin->setEnabled(false);
					shortcutBet->setEnabled(true);
					shortcutRaise->setEnabled(true);
				}
				
				stlayActions->setCurrentIndex(m_nActions);
				
				if (wRaiseBtns->isVisible())
				{
					const chips_type cur_pot = currentPot();
					
					btnBetsizeMinimum->setEnabled(s->stake + s->bet > snap->minimum_bet);
					btnBetsizeQuarterPot->setEnabled(cur_pot * 0.25f > snap->minimum_bet);
					btnBetsizeHalfPot->setEnabled(cur_pot * 0.5f > snap->minimum_bet);
					btnBetsizeThreeQuarterPot->setEnabled(cur_pot * 0.75f > snap->minimum_bet);
					btnBetsizePotsize->setEnabled(cur_pot >= snap->minimum_bet);
				}
			}
		}
		else
		{
			if (s->stake == 0 || (!bGreaterBet && snap->s_lastbet == snap->my_seat))  // FIXME: do not show actions if there is no action possible for this betting round
				stlayActions->setCurrentIndex(m_nNoAction);
			else
			{
				if (bGreaterBet)
				{
					chkAutoFoldCheck->setText(tr("Fold"));
					
					if (greatest_bet >= s->stake + s->bet)
						chkAutoCheckCall->setText(tr("Allin %1").arg(s->stake));
					else
						chkAutoCheckCall->setText(tr("Call %1").arg(greatest_bet - s->bet));
				}
				else
				{
					chkAutoFoldCheck->setText(tr("Check/Fold"));
					chkAutoCheckCall->setText(tr("Check"));
				}
				
				stlayActions->setCurrentIndex(m_nPreActions);
			}
		}
	}
	
	
	// evaluate persistent actions
	if (s->sitout)
		stlayPersistentActions->setCurrentIndex(m_nBack);
	else
		stlayPersistentActions->setCurrentIndex(m_nSitout);
}

unsigned int WTable::seatToCentralView(int my, unsigned int seat) const
{
	int mapped_seat;
	if (my != -1 && config.getBool("ui_centralized_view"))
	{
		mapped_seat = (seat - my) + 4;
		if (mapped_seat >= (int)nMaxSeats)
			mapped_seat -= nMaxSeats;
		else if (mapped_seat < 0)
			mapped_seat += nMaxSeats;
	}
	else
		mapped_seat = seat;
	
	return mapped_seat;
}

void WTable::updateSeat(unsigned int s)
{
	const gameinfo *ginfo = ((PClient*)qApp)->getGameInfo(m_nGid);
	if (!ginfo)
		return;
	
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;

	const table_snapshot *snap = &(tinfo->snap);
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	const seatinfo *seat = &(snap->seats[s]);
	Q_ASSERT_X(seat, Q_FUNC_INFO, "invalid seat pointer");
	
	
	// get correct mapping (central/normal view) for seat-id
	const unsigned int mapped_seat = seatToCentralView(snap->my_seat, s);
	
	// pointer to seat-entity
	Seat *ui_seat = wseats[mapped_seat];
	
	ui_seat->setValid(seat->valid);
	
	// update seat widget if seat is occupied
	if (seat->valid)
	{
		ui_seat->setStake(seat->stake);
		ui_seat->setSitout(seat->sitout);
		ui_seat->setInfo(
			((PClient*)qApp)->playerList()->name(seat->client_id),
			((PClient*)qApp)->playerList()->location(seat->client_id));
		
		if (snap->state > Table::ElectDealer)
		{
			// highlight current seat
			ui_seat->setCurrent(snap->s_cur != -1 && snap->s_cur == (int)s);
			
			// update timeout display
			if (snap->s_cur != -1 &&
				(snap->state == Table::Blinds || 
				snap->state == Table::Betting) &&
				snap->seats[snap->s_cur].stake > 0 &&
				snap->seats[snap->s_cur].sitout == false)
			{
				const unsigned int curseat_mapped = seatToCentralView(snap->my_seat, snap->s_cur);
				m_pTimeout->setPos(calcTimeoutPos(curseat_mapped));
				m_pTimeout->start(snap->s_cur, ginfo->player_timeout);
				m_pTimeout->show();
			}
			else
			{
				m_pTimeout->hide();
				m_pTimeout->stop();
			}
		}
		
		// in case the player associated with seat is involved in hand
		if (seat->in_round)
		{
			// is it our seat?
			ui_seat->setMySeat(snap->my_seat == (int)s);
			
			
			if (snap->state == Table::EndRound)
				ui_seat->setWin(seat->bet);
			else
				ui_seat->setAction((seat->stake == 0) ? Player::Allin : seat->action,
								seat->bet);
			
			
			
			std::vector<Card> allcards;
			
			// use cards of saved holecards or snapshot
			if (snap->my_seat == (int)s)
				tinfo->holecards.copyCards(&allcards);
			else
				seat->holecards.copyCards(&allcards);
			
			// are there cards visible?
			if (allcards.size())
			{
				char card1[3], card2[3];
				strcpy(card1, allcards[0].getName());
				strcpy(card2, allcards[1].getName());
				
				// display the holecards sorted?
				if (config.getBool("ui_sort_holecards"))
				{
					Card c1(card1), c2(card2);
					
					// sort cards descending
					if (c1 < c2)
						ui_seat->setCards(card2, card1);
					else
						ui_seat->setCards(card1, card2);
				}
				else
					ui_seat->setCards(card1, card2);
				
				ui_seat->showBigCards(true);
			}
			else
			{
				if (snap->my_seat == (int)s)
					ui_seat->setCards("blank", "blank");
				else
					ui_seat->setCards("back", "back");
				
				ui_seat->showBigCards(false);
			}
			
			ui_seat->showSmallCards(true);
		}
		else   // player isn't anymore involved in current hand
		{
			ui_seat->setAction(seat->action);
			ui_seat->setCards("blank", "blank");
			ui_seat->showBigCards(false);
			ui_seat->showSmallCards(false);
		}
	}
	
	
	// schedule scene update
	ui_seat->update(ui_seat->boundingRect());
}

void WTable::updatePots()
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;

	const table_snapshot *snap = &(tinfo->snap);
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	for (unsigned int t = 0; t < sizeof(m_Pots) / sizeof(m_Pots[0]); t++)
		m_Pots[t]->hide();
	
	QString strPots;
	chips_type pot_sum = 0;
	if (snap->pots.at(0) > 0)
	{
		pot_sum += snap->pots.at(0);
		
		strPots = QString(tr("Main pot: %1").arg(snap->pots.at(0)));
		for (unsigned int t = 1; t < snap->pots.size(); ++t)
		{
			pot_sum += snap->pots.at(t);
			
			strPots.append(
				QString("  " + tr("Side pot %1: %2")
					.arg(t).arg(snap->pots.at(t))));
					
			m_Pots[t]->setAmount(snap->pots.at(t));
			m_Pots[t]->show();
		}
	}
	
	QString strPotSum;
	if (snap->state <= Table::Betting && currentPot() > 0)
		strPotSum = QString("%1").arg(currentPot());
	else if (pot_sum > 0)
		strPotSum = QString("%1").arg(pot_sum);
	
	m_pTxtPots->setText(strPotSum);
	m_pTxtPots->setToolTip(strPots);
	m_pTxtPots->setPos(calcTxtPotsPos());
	
	m_Pots[0]->setAmount(snap->pots.at(0));
	m_Pots[0]->show();
	// 
	calcPotsPos();
}

void WTable::updateDealerButton()
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;

	const table_snapshot *snap = &(tinfo->snap);
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	
	// remember last state change
	static int last_state = -1;
	
	if (snap->state == Table::NewRound && last_state != snap->state)
	{
		// update dealer button
		const unsigned int dealerseat_mapped = seatToCentralView(snap->my_seat, snap->s_dealer);
		m_pDealerButton->startAnimation(m_ptDealerBtn[dealerseat_mapped]);
	}
	
	last_state = snap->state;
}

void WTable::updateCommunityCards()
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;

	const table_snapshot *snap = &(tinfo->snap);
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	
	if (snap->state == Table::NewRound)
		for (unsigned int i = 0; i < 5; i++)
			m_CommunityCards[i]->hide();
	
	
	std::vector<Card> allcards;
	snap->communitycards.copyCards(&allcards);
	
	
	// load community cards   // FIXME: cache pixmaps
	for (unsigned int i = 0; i < allcards.size(); i++)
	{
		m_CommunityCards[i]->setPixmap(
			QPixmap(
				QString("gfx/deck/%1/%2.png")
					.arg(QString::fromStdString(config.get("ui_card_deck")))
					.arg(allcards[i].getName())));
		m_CommunityCards[i]->show();
	}
}

void WTable::handleAutoActions()
{	
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;

	const table_snapshot *snap = &(tinfo->snap);
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	if (snap->my_seat == -1)
		return;
	
	const seatinfo *seat = &(snap->seats[snap->my_seat]);
	Q_ASSERT_X(seat, Q_FUNC_INFO, "invalid seat pointer");
	
	
	// handle auto-actions
	if (snap->state == Table::Betting)
	{
		if ((int)snap->s_cur == snap->my_seat)
		{
			if (chkAutoFoldCheck->checkState() == Qt::Checked)
			{
				if (greaterBet(snap, seat->bet))
					actionFold();
				else
					actionCheckCall();
				
				chkAutoFoldCheck->setCheckState(Qt::Unchecked);
			}
			else if (chkAutoCheckCall->checkState() == Qt::Checked)
			{
				chips_type greatest_bet;
				greaterBet(snap, 0, &greatest_bet);
				
				if (m_autocall_amount >= greatest_bet)
					actionCheckCall();
				
				chkAutoCheckCall->setCheckState(Qt::Unchecked);
			}
		}
		else  // validate pre-actions
		{
			if (chkAutoCheckCall->checkState() == Qt::Checked)
			{
				chips_type greatest_bet;
				greaterBet(snap, 0, &greatest_bet);
				
				if (m_autocall_amount < greatest_bet)
					chkAutoCheckCall->setCheckState(Qt::Unchecked);
			}
		}
	}
	else
	{
		// reset all actions (pre-caution)
		chkAutoFoldCheck->setCheckState(Qt::Unchecked);
		chkAutoCheckCall->setCheckState(Qt::Unchecked);
	}
}

void WTable::updateHandStrength()
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;

	const table_snapshot *snap = &(tinfo->snap);
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	// is not a player
	if (snap->my_seat == -1)
	{
		m_pTxtHandStrength->setText(QString());
		return;
	}
	
	const seatinfo *seat = &(snap->seats[snap->my_seat]);
	Q_ASSERT_X(seat, Q_FUNC_INFO, "invalid seat pointer");
	
	
	if (seat->in_round)
	{
		vector<Card> allcards;
		tinfo->holecards.copyCards(&allcards);
		
		if (allcards.size())
		{
			HandStrength strength;
			GameLogic::getStrength(&(tinfo->holecards), &(snap->communitycards), &strength);
			
			m_pTxtHandStrength->setText(WTable::buildHandStrengthString(&strength, 0));
			m_pTxtHandStrength->setPos(calcHandStrengthPos());
		}
		else
			m_pTxtHandStrength->setText(QString());
	}
	else
		m_pTxtHandStrength->setText(QString());
}

void WTable::updateView()
{
	const gameinfo *ginfo = ((PClient*)qApp)->getGameInfo(m_nGid);
	if (!ginfo)
		return;
	
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;

	const table_snapshot *snap = &(tinfo->snap);
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	
	
	// update seat widgets
	for (unsigned int i=0; i < nMaxSeats; i++)
		updateSeat(i);
	
	
	// update the dealer button
	updateDealerButton();
	
	
	// update all pots
	updatePots();
	
	
	// update hand-strength
	updateHandStrength();
	
	
	// update the community cards
	updateCommunityCards();
	
	
	// evaluate all available actions
	evaluateActions(snap);
	
	
	// handle pre-set actions
	handleAutoActions();
	
	
	// set focus on EditableSlider only if focus isn't on ChatBox
	if (!m_pChat->hasInputFocus())
		m_pSliderAmount->setFocus();
}

void WTable::addChat(const QString& from, const QString& text)
{
	m_pChat->addMessage(text, from);
	
	if (config.getBool("log_chat"))
		log_msg("table", "(%d:%d) (%s) %s",
			m_nGid,
			m_nTid,
			from.toStdString().c_str(),
			text.toStdString().c_str());
}

void WTable::addServerMessage(const QString& text)
{
	m_pChat->addMessage(text, Qt::blue);
	
	if (config.getBool("log_chat"))
		log_msg("table", "(%d:%d) %s",
			m_nGid,
			m_nTid,
			text.toStdString().c_str());
}

void WTable::closeEvent(QCloseEvent *event)
{
	// FIXME: handle this case: send sitout if player (my_seat != -1 && sitout == false)
	
	// store settings
	QSettings settings;

	settings.beginGroup("TableWindow");
		settings.setValue("size", this->size());
		settings.setValue("pos", this->pos());
	settings.endGroup();
}

void WTable::actionFold()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Fold);
	
	stlayActions->setCurrentIndex(m_nNoAction);
}

void WTable::actionCheckCall()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Call);
	
	stlayActions->setCurrentIndex(m_nNoAction);
}

void WTable::actionBetRaise()
{
	tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	Q_ASSERT_X(tinfo, Q_FUNC_INFO, "getTableInfo failed");
	
	table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");

	if (snap->my_seat == -1)
		return;
	
	chips_type greatest_bet = 0;
	greaterBet(snap, 0, &greatest_bet);
	
	if (greatest_bet >= snap->seats[snap->my_seat].stake + snap->seats[snap->my_seat].bet ||
		snap->minimum_bet >= snap->seats[snap->my_seat].stake + snap->seats[snap->my_seat].bet)
	{
		((PClient*)qApp)->doSetAction(m_nGid, Player::Allin);
	}
	else
		((PClient*)qApp)->doSetAction(m_nGid, Player::Raise, m_pSliderAmount->value());
	
	stlayActions->setCurrentIndex(m_nNoAction);
}

void WTable::actionShow()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Show);
	
	stlayActions->setCurrentIndex(m_nNoAction);
}

void WTable::actionMuck()
{
	((PClient*)qApp)->doSetAction(m_nGid, Player::Muck);
	
	stlayActions->setCurrentIndex(m_nNoAction);
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
	evaluateActions(snap);
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

void WTable::actionAutoFoldCheck(int state)
{
	// uncheck other auto-actions
	if (state == Qt::Checked)
		chkAutoCheckCall->setCheckState(Qt::Unchecked);
}

void WTable::actionAutoCheckCall(int state)
{
	if (state != Qt::Checked)
		return;
	
	// uncheck other auto-actions
	chkAutoFoldCheck->setCheckState(Qt::Unchecked);
	
	
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	if (!tinfo)
		return;
	
	const table_snapshot *snap = &(tinfo->snap);
	if (!snap)
		return;
	
	chips_type greatest_bet;
	greaterBet(snap, 0, &greatest_bet);
	
	m_autocall_amount = greatest_bet;
}

void WTable::slotBetRaiseAmountChanged()
{
	const bool is_valid = m_pSliderAmount->validValue();
	btnBetRaise->setEnabled(is_valid);
	shortcutRaise->setEnabled(is_valid);
	shortcutBet->setEnabled(is_valid);
	
	const QString str = btnBetRaise->text();
	
	btnBetRaise->setText(QString("%1 %2")
				.arg(str.left(str.lastIndexOf(' ')))
				.arg(m_pSliderAmount->value()));
}

chips_type WTable::currentPot() const
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	Q_ASSERT_X(tinfo, Q_FUNC_INFO, "getTableInfo failed");
	
	const table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");

	chips_type cur_pot = 0;
	
	// sum all pots
	for (unsigned int i=0; i < snap->pots.size(); i++)
		cur_pot += snap->pots.at(i);
	
	// sum all seat bets
	for (unsigned int i=0; i < nMaxSeats; i++)
	{
		const seatinfo *seat = &(snap->seats[i]);

		if (seat->valid && seat->in_round)
			cur_pot += seat->bet;
	}

	return cur_pot;
}

void WTable::actionBetsizeMinimum()
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	Q_ASSERT_X(tinfo, Q_FUNC_INFO, "getTableInfo failed");
	
	const table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	if (snap->my_seat == -1)
		return;
	
	m_pSliderAmount->setValue(snap->minimum_bet);
}

void WTable::actionBetsizeQuarterPot()
{
	m_pSliderAmount->setValue(int(currentPot() * 0.25f));
}

void WTable::actionBetsizeHalfPot()
{
	m_pSliderAmount->setValue(int(currentPot() * 0.5f));
}

void WTable::actionBetsizeThreeQuarterPot()
{
	m_pSliderAmount->setValue(int(currentPot() * 0.75f));
}

void WTable::actionBetsizePotsize()
{
	m_pSliderAmount->setValue(currentPot());
}

void WTable::actionBetsizeMaximum()
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	Q_ASSERT_X(tinfo, Q_FUNC_INFO, "getTableInfo failed");
	
	const table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");
	
	if (snap->my_seat == -1)
		return;
	
	m_pSliderAmount->setValue(snap->seats[snap->my_seat].stake + snap->seats[snap->my_seat].bet);
}

void WTable::slotShow()
{
	updateView();
	
	show();
	
	// FIXME: better solution
	resizeEvent(NULL);
}

void WTable::slotTimeup(int seat)
{
	qDebug() << "timeup seat= " << seat;
}

void WTable::slotFirstReminder(int seatnr)
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	if (!tinfo)
		return;
	
	const table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");

	if (snap->my_seat != seatnr)
		return;

	const seatinfo *seat = &(snap->seats[seatnr]);

	Q_ASSERT_X(seat, Q_FUNC_INFO, "invalid seat pointer");
		
	addServerMessage(
		QString(tr("%1, it's your turn!")
			.arg(((PClient*)qApp)->playerList()->name(seat->client_id))));
}

void WTable::slotSecondReminder(int seatnr)
{
	const tableinfo *tinfo = ((PClient*)qApp)->getTableInfo(m_nGid, m_nTid);
	
	if (!tinfo)
		return;
	
	const table_snapshot *snap = &(tinfo->snap);
	
	Q_ASSERT_X(snap, Q_FUNC_INFO, "invalid snapshot pointer");

	if (snap->my_seat != seatnr)
		return;

	const seatinfo *seat = &(snap->seats[seatnr]);

	Q_ASSERT_X(seat, Q_FUNC_INFO, "invalid seat pointer");
		
	const gameinfo *ginfo = ((PClient*)qApp)->getGameInfo(m_nGid);

	Q_ASSERT_X(ginfo, Q_FUNC_INFO, "invalid gameinfo pointer");
		
	addServerMessage(
		QString(tr("%1, you have %2 seconds left to respond!")
			.arg(((PClient*)qApp)->playerList()->name(seat->client_id))
			.arg(ginfo->player_timeout - ginfo->player_timeout / 4 * 3)));

	// additionally play sound
	playSound(SOUND_REMINDER_1);
}

void WTable::resizeEvent(QResizeEvent *event)
{
	// preserve aspect ratio of our view
	const float aspect_ratio = 0.6f;
	int new_width = (int)(m_pView->height() / aspect_ratio);
	
	// fit in window if preserving aspect-ratio is not possible
	if (new_width > this->width())
		new_width = this->width();
	
	m_pView->resize(new_width, m_pView->height());
	m_pView->move(width()/2 - m_pView->width() / 2, m_pView->y());
	
	m_pView->fitInView(m_pScene->itemsBoundingRect());
}

bool WTable::greaterBet(const table_snapshot *snap, const chips_type bet, chips_type *pbet) const
{
	chips_type cur_bet = bet;
	
	for (unsigned int i=0; i < nMaxSeats; i++)
	{
		const seatinfo *seat = &(snap->seats[i]);

		if (seat->valid && seat->in_round && seat->bet > cur_bet)
			cur_bet = seat->bet;
	}
	
	if (pbet)
		*pbet = cur_bet;
	
	return (cur_bet > bet);
}

void WTable::actionScreenshot()
{
	QString pathScrshot = QString(sys_config_path()) + "/screenshots";
	QDateTime datetime = QDateTime::currentDateTime();
	QString filename = QString("holdingnuts_%1.png")
		.arg(datetime.toString("yyyy-MM-dd_hh.mm.ss"));
	
	QDir dir;
	if (!dir.exists(pathScrshot))
		dir.mkdir(pathScrshot);
	
	// grab the content of this window
	QPixmap pixShot = QPixmap::grabWidget(this);
	
	if (pixShot.save(pathScrshot + "/" + filename, "PNG"))
		addServerMessage(tr("Saved screenshot: %1.").arg(filename));
	else
		addServerMessage(tr("Unable to save screenshot in %1.").arg(pathScrshot));
}

void WTable::actionChat(QString msg)
{
	((PClient*)qApp)->chat(msg, m_nGid, m_nTid);
}

void WTable::playSound(unsigned int id) const
{
#ifndef NOAUDIO
	if (!config.getBool("sound") || isHidden() || (config.getBool("sound_focus") && !isActiveWindow()))
		return;
	
	audio_play(id);
#endif
}

QString WTable::buildSuitString(const Card& card)
{
	QString scard;
	
	switch (card.getSuit())
	{
		case Card::Spades:	scard = tr("Spades");	break;
		case Card::Hearts:	scard = tr("Hearts");	break;
		case Card::Diamonds:	scard = tr("Diamonds");	break;
		case Card::Clubs:	scard = tr("Clubs");	break;
	}
	
	return scard;
}

QString WTable::buildFaceString(const Card& card, bool plural)
{
	QString scard;
	
	switch (card.getFace())
	{
		case Card::Two:		scard = !plural ? tr("Deuce") :	tr("Deuces");	break;
		case Card::Three:	scard = !plural ? tr("Three") :	tr("Threes");	break;
		case Card::Four:	scard = !plural ? tr("Four") :	tr("Fours");	break;
		case Card::Five:	scard = !plural ? tr("Five") :	tr("Fives");	break;
		case Card::Six:		scard = !plural ? tr("Six") :	tr("Sixes");	break;
		case Card::Seven:	scard = !plural ? tr("Seven") :	tr("Sevens");	break;
		case Card::Eight:	scard = !plural ? tr("Eight") :	tr("Eights");	break;
		case Card::Nine:	scard = !plural ? tr("Nine") :	tr("Nines");	break;
		case Card::Ten:		scard = !plural ? tr("Ten") :	tr("Tens");	break;
		case Card::Jack:	scard = !plural ? tr("Jack") :	tr("Jacks");	break;
		case Card::Queen:	scard = !plural ? tr("Queen") :	tr("Queens");	break;
		case Card::King:	scard = !plural ? tr("King") :	tr("Kings");	break;
		case Card::Ace:		scard = !plural ? tr("Ace") :	tr("Aces");	break;
	}
	
	return scard;
}

QString WTable::buildHandStrengthString(HandStrength *strength, int verbosity)
{
	vector<Card> rank, kicker;
	strength->copyRankCards(&rank);
	strength->copyKickerCards(&kicker);
	
	// provide translation for hand strength
	QString sstrength = "unknown hand strength";
	QString srank;
	QStringList slkicker;
	
	for (unsigned int i=0; i < kicker.size(); i++)
		slkicker += QString(kicker[i].getName());
	
	switch (strength->getRanking())
	{
	case HandStrength::HighCard:
		sstrength = tr("High Card");
		srank = QString("%1").arg(buildFaceString(rank[0]));
		break;
	case HandStrength::OnePair:
		sstrength = tr("One Pair");
		srank = QString("%1").arg(buildFaceString(rank[0], true));
		break;
	case HandStrength::TwoPair:
		sstrength = tr("Two Pair");
		srank = tr("%1 and %2").arg(buildFaceString(rank[0], true)).arg(buildFaceString(rank[1], true));
		break;
	case HandStrength::ThreeOfAKind:
		sstrength = tr("Three Of A Kind");
		srank = QString("%1").arg(buildFaceString(rank[0], true));
		break;
	case HandStrength::Straight:
		sstrength = tr("Straight");
		srank = tr("%1 high").arg(buildFaceString(rank[0]));
		break;
	case HandStrength::Flush:
		sstrength = tr("Flush");
		srank = QString("%1").arg(buildSuitString(rank[0]));
		
		if (verbosity)
		{
			QStringList slrank;
			
			for (unsigned int i=0; i < rank.size(); i++)
				slrank += QString(rank[i].getName());
			
			srank += " (" + slrank.join(" ") + ")";
		}
		break;
	case HandStrength::FullHouse:
		sstrength = tr("Full House");
		srank = tr("%1 and %2").arg(buildFaceString(rank[0], true)).arg(buildFaceString(rank[1], true));
		break;
	case HandStrength::FourOfAKind:
		sstrength = tr("Four Of A Kind");
		srank = QString("%1").arg(buildFaceString(rank[0], true));
		break;
	case HandStrength::StraightFlush:
		// handle RoyalFlush as special case
		if (strength->getRanking() == HandStrength::StraightFlush && rank.front().getFace() == Card::Ace)
		{
			sstrength = tr("Royal Flush");
			srank = QString("%1").arg(buildSuitString(rank[0]));
		}
		else
		{
			sstrength = tr("Straight Flush");
			srank = QString("%1 ").arg(buildSuitString(rank[0])) +
				tr("%1 high").arg(buildFaceString(rank[0]));
		}
		break;
	}
	
	QString retstr = sstrength + ", " + srank;
	
	if (verbosity && kicker.size())
		retstr += " (" + slkicker.join(" ") + ")";
	
	return retstr;
}

void WTable::showDebugTable()
{
#ifdef DEBUG
	qsrand(QDateTime::currentDateTime().toTime_t());

	m_pDealerButton->hide();

	DealerButton *dealerBtn[nMaxSeats];
	
	// seats and dealerbutton
	for (unsigned int i = 0; i < nMaxSeats; i++)
	{
		wseats[i]->setAction(
			Player::PlayerAction(qrand() % Player::Sitout),
			(qrand() % 30000 + 1));
		wseats[i]->setInfo(
			QString::fromStdString(config.get("player_name")),
			QString::fromStdString(config.get("info_location")));
		wseats[i]->setStake(10000);
		wseats[i]->setValid(true);
		wseats[i]->setSitout(bool(qrand()%2));
		wseats[i]->showBigCards(true);
		wseats[i]->showSmallCards(true);
		wseats[i]->setCards("As", "7h");

		// dealerbutton
		dealerBtn[i] = new DealerButton;
		dealerBtn[i]->setPos(m_ptDealerBtn[i]);

		m_pScene->addItem(dealerBtn[i]);	
	}
	
	// mainpot and sidepots
//	for (unsigned int t = 0; t < qrand() % (sizeof(m_Pots) / sizeof(m_Pots[0])) + 1; t++)
	for (unsigned int t = 0; t < sizeof(m_Pots) / sizeof(m_Pots[0]); t++)
		m_Pots[t]->setAmount(qrand() % 30000 + 1);
	calcPotsPos();
	
	// timeout
	m_pTimeout->setPos(calcTimeoutPos(0));
	m_pTimeout->start(0, 60);
	m_pTimeout->show();

	// community cards
	for (unsigned int j = 0; j < 5; j++)
	{
		m_CommunityCards[j]->setPixmap(
			QPixmap(
				QString("gfx/deck/%1/Ac.png")
					.arg(QString::fromStdString(config.get("ui_card_deck")))));
		m_CommunityCards[j]->show();
	}
#endif /* DEBUG */
}

#if defined(Q_OS_WIN)
#	include <windows.h>
#endif

void WTable::setForegroundWindow()
{
	if (!config.getBool("ui_bring_on_top"))
		return;

	this->activateWindow();
	this->raise();
	
#if defined(Q_OS_WIN)
	SetWindowPos(winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif

// TODO: test on other platforms
}

