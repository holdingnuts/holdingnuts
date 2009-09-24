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


#ifndef _WTABLE_H
#define _WTABLE_H

#include <vector>

#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QButtonGroup>
#include <QPushButton>
#include <QCheckBox>
#include <QPalette>
#include <QGraphicsView>
#include <QGraphicsItemAnimation>

#include "Card.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "GameLogic.hpp"
#include "Table.hpp"
#include "Player.hpp"
#include "Seat.hpp"

typedef struct {
	bool valid;
	int client_id;
	chips_type bet;
	chips_type stake;
	bool in_round;
	bool sitout;
	Player::PlayerAction action;
	HoleCards holecards;
} seatinfo;

typedef struct {
	int state;
	int betting_round;
	int s_dealer;
	int s_sb;
	int s_bb;
	int s_cur;
	int s_lastbet;
	std::vector<chips_type> pots;
	chips_type minimum_bet;
	CommunityCards communitycards;
	seatinfo seats[10];
	int my_seat;
	bool nomoreaction;
} table_snapshot;

class QGraphicsView;
class QGraphicsScene;
class QStackedLayout;
class QLabel;
class QSlider;
class QShortcut;
class QPushButton;
class QCheckBox;

class ChatBox;
class DealerButton;
class EditableSlider;
class TimeOut;
class ChipStack;

class WTable : public QWidget
{
Q_OBJECT

public:
	WTable(int gid, int tid, QWidget *parent = 0);
	~WTable();
	
	//! \brief Update whole table view
	void updateView();
	

	void addChat(const QString& from, const QString& text);
	void addServerMessage(const QString& text);
	
	void playSound(unsigned int id) const;
	
	//! \brief Translate seat-number depending on view-point
	unsigned int seatToCentralView(int my, unsigned int seat) const;
	
	static const unsigned int	nMaxSeats;
	
	static QString buildHandStrengthString(HandStrength *strength, int verbosity=0);
	static QString buildFaceString(const Card& card, bool plural=false);
	static QString buildSuitString(const Card& card);
	
	//! \brief shows all entitys from table
	void showDebugTable();
	
	//! \brief brings window to foreground
	void setForegroundWindow();

protected:
	void closeEvent(QCloseEvent *event);
	void resizeEvent(QResizeEvent *event);
	
	QPointF calcSeatPos(unsigned int nSeatID) const;
	QPointF calcCCardsPos(unsigned int nCard) const;
	QPointF calcTimeoutPos(unsigned int nSeatID) const;
	QPointF calcHandStrengthPos() const;
	QPointF calcTxtPotsPos() const;
	QPointF calcDealerBtnPos(unsigned int nSeatID) const;
	void calcPotsPos();
	
	void doSitout(bool bSitout);

	void evaluateActions(const table_snapshot *snap);
	
	bool greaterBet(
		const table_snapshot *snap,
		const chips_type bet,
		chips_type *pbet = 0) const;
	

	//! \brief returns current Potsize including all Bets on Table
	chips_type currentPot() const;
	
	void updateSeat(unsigned int s);
	void updatePots();
	void updateDealerButton();
	void updateCommunityCards();
	void updateHandStrength();
	
	void handleAutoActions();

private slots:
	void actionFold();
	void actionCheckCall();
	void actionBetRaise();
	void actionShow();
	void actionMuck();
	void actionBack();
	void actionSitout();
	
	void actionAutoFoldCheck(int state);
	void actionAutoCheckCall(int state);

	void slotTimeup(int seat);
	void slotFirstReminder(int seat);
	void slotSecondReminder(int seat);

	void actionScreenshot();
	
	void actionChat(QString msg);
	
	void slotBetRaiseAmountChanged();
	
	void actionBetsizeMinimum();
	void actionBetsizeQuarterPot();
	void actionBetsizeHalfPot();
	void actionBetsizeThreeQuarterPot();
	void actionBetsizePotsize();
	void actionBetsizeMaximum();

public slots:
	void slotShow();

private:
	//! \brief Game ID
	const int	m_nGid;
	//! \brief Table ID
	const int	m_nTid;
	
	QGraphicsView			*m_pView;
	QGraphicsScene			*m_pScene;

	// scene items
	QGraphicsPixmapItem		*m_pImgTable;
	DealerButton			*m_pDealerButton;
	Seat					*wseats[10];
	QGraphicsPixmapItem		*m_CommunityCards[5];
	TimeOut					*m_pTimeout;
	QGraphicsSimpleTextItem *m_pTxtPots;
	QGraphicsSimpleTextItem *m_pTxtHandStrength;
	ChipStack				*m_Pots[8];
	
	// ui
	ChatBox			*m_pChat;
	QStackedLayout	*stlayActions;
	EditableSlider	*m_pSliderAmount;
	QPushButton		*btnCheckCall;
	QPushButton		*btnBetRaise;
	
	QPushButton		*btnBetsizeMinimum;
	QPushButton		*btnBetsizeQuarterPot;
	QPushButton		*btnBetsizeHalfPot;
	QPushButton		*btnBetsizeThreeQuarterPot;
	QPushButton		*btnBetsizePotsize;
	QPushButton		*btnBetsizeMaximum;
	QWidget			*wRaiseBtns;
	
	QCheckBox 		*chkAutoFoldCheck;
	QCheckBox		*chkAutoCheckCall;
	
	// stackedlayout widgets id
	int				m_nActions;
	int				m_nPreActions;
	int				m_nPostActions;
	int				m_nNoAction;
	
	
	QLabel		*lblPersistentActions;
	QStackedLayout	*stlayPersistentActions;
	int				m_nSitout;
	int				m_nBack;
	
	chips_type		m_autocall_amount;
	
	// shortcuts
	QShortcut		*shortcutFold;
	QShortcut		*shortcutCallCheck;
	QShortcut		*shortcutBet;
	QShortcut		*shortcutRaise;
	QShortcut		*shortcutAllin;
	QShortcut		*shortcutMuck;
	QShortcut		*shortcutShow;
	QShortcut		*shortcutSitout;
	QShortcut		*shortcutBack;
	
	// 
	QSizeF			sizeCommunityCards;
	
	qreal			posYCommunityCards;
	qreal			posYTxtPots;
	qreal			posYPots;
	// precomputed dealerbtn positions
	QPointF			m_ptDealerBtn[10];
};

#endif /* _WTABLE_H */
