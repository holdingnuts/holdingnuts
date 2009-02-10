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
#include <QList>

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

class ChatBox;
class DealerButton;
class EditableSlider;

class WTable : public QGraphicsView
{
Q_OBJECT

public:
	WTable(int gid, int tid, QWidget *parent = 0);
	
	void updateView();

	void addChat(const QString& from, const QString& text);
	void addServerMessage(const QString& text);
	
public:
	static const unsigned int	nMaxSeats;

protected:
	void closeEvent(QCloseEvent *event);
	void resizeEvent(QResizeEvent *event);
	
	QPointF calcSeatPos(unsigned int nSeatID) const;
	QPointF calcCCardsPos(
		unsigned int nCard,
		int table_width) const;
	
	void doSitout(bool bSitout);

	void evaluateActions(const table_snapshot *snap);
	
	bool greaterBet(
		const table_snapshot *snap,
		const qreal& bet,
		qreal *pbet = 0) const;

private slots:
	void actionFold();
	void actionCheckCall();
	void actionBetRaise();
	void actionShow();
	void actionMuck();
	void actionBack();
	void actionSitout();
	
	void actionAutoCheckCall(int state);

	void slotShow();
	
	
private:
	//! \brief Game ID
	const int	m_nGid;
	//! \brief Table ID
	const int	m_nTid;

	//! \brief Tableimage
	QGraphicsPixmapItem		*m_pImgTable;
	//! \brief Dealer Button
	DealerButton			*m_pDealerButton;
	//! \brief Seats
	Seat					*wseats[10];
	//! \brief
	QGraphicsPixmapItem		*m_CommunityCards[5];
	//! \brief Timeout
	QTimeLine				m_timeLine;
	//! \brief Image Timeout
	QGraphicsPixmapItem		*m_pImgTimeout;
	//! \brief shear Animation from Timeout
	QGraphicsItemAnimation	m_animTimeout;
	
	// ui
	ChatBox			*m_pChat;
	QLabel			*m_LayoutActions;
	QStackedLayout		*stlayActions;
	QLabel			*lblPots;
	QLabel			*lblHandStrength;
	EditableSlider		*m_pSliderAmount;
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
	
	qreal				m_autocall_amount;
};

#endif /* _WTABLE_H */
