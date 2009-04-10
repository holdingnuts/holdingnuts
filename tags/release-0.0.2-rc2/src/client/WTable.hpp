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
#include <QGridLayout>	// TODO: remove!!! only wmain needs this
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
	float bet;
	float stake;
	bool in_round;
	bool sitout;
	Player::PlayerAction action;
	HoleCards holecards;
} seatinfo;

typedef struct {
	int state;
	int betting_round;
	unsigned int s_dealer;
	unsigned int s_sb;
	unsigned int s_bb;
	unsigned int s_cur;
	unsigned int s_lastbet;
	std::vector<float> pots;
	float minimum_bet;
	CommunityCards communitycards;
	seatinfo seats[10];
	int my_seat;
} table_snapshot;


class QStackedLayout;
class QLabel;
class QSlider;
class QShortcut;

class ChatBox;
class DealerButton;
class EditableSlider;
class TimeOut;

class WTable : public QWidget
{
Q_OBJECT

public:
	WTable(int gid, int tid, QWidget *parent = 0);
	~WTable();
	
	void updateView();

	void addChat(const QString& from, const QString& text);
	void addServerMessage(const QString& text);
	
	static const unsigned int	nMaxSeats;

protected:
	void closeEvent(QCloseEvent *event);
	void resizeEvent(QResizeEvent *event);
	
	QPointF calcSeatPos(unsigned int nSeatID) const;
	QPointF calcCCardsPos(unsigned int nCard) const;
	QPointF calcTimeoutPos(unsigned int nSeatID) const;
	QPointF calcHandStrengthPos() const;
	QPointF calcPotsPos() const;
	QPointF calcDealerBtnPos(
		unsigned int nSeatID, 
		int offset = 20) const;
	
	void doSitout(bool bSitout);

	void evaluateActions(const table_snapshot *snap);
	
#if 0
	bool isNoMoreActionPossible(const table_snapshot *snap);
#endif
	
	bool greaterBet(
		const table_snapshot *snap,
		const qreal& bet,
		qreal *pbet = 0) const;
	
	void playSound(unsigned int id);

private slots:
	void actionFold();
	void actionCheckCall();
	void actionBetRaise();
	void actionShow();
	void actionMuck();
	void actionBack();
	void actionSitout();
	void actionAutoCheckCall(int state);

	void slotTimeup(int seat);
	
	void actionScreenshot();
	
	void actionChat(QString msg);
	
	void slotBetRaiseAmountChanged();

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
	
	// ui
	ChatBox			*m_pChat;
	QStackedLayout	*stlayActions;
	EditableSlider	*m_pSliderAmount;
	QPushButton		*btnCheckCall;
	QPushButton		*btnBetRaise;
	QCheckBox 		*chkAutoFoldCheck;
	QCheckBox		*chkAutoCheckCall;
	
	// stackedlayout widgets id
	int				m_nActions;
	int				m_nPreActions;
	int				m_nPostActions;
	int				m_nNoAction;
	int				m_nSitoutActions;
	
	qreal			m_autocall_amount;
	
	// precomputed dealerbtn positions
	QPointF			m_ptDealerBtn[10];
	
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
};

#endif /* _WTABLE_H */