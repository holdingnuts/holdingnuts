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
#include <QCheckBox>
#include <QPalette>
#include <QGridLayout>	// TODO: remove!!! only wmain needs this
#include <QGraphicsView>

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

// class Seat;
class QStackedLayout;
class QLabel;
class QSlider;

class ChatBox;
class DealerButton;

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
	void resizeEvent(QResizeEvent * event);
	
	QPointF calcSeatPos(unsigned int nSeatID);
	QPointF calcDealerBtnPos(unsigned int nSeatID);

private slots:
	void actionFold();
	void actionCheckCall();
	void actionBetRaise();
	void actionShow();
	void actionMuck();
	void slotBetValue(int value);
	void slotShow();
	void slotTest();

private:
	QStackedLayout *stlayActions;
	QLineEdit *editAmount;
	QSlider *sliderAmount;
	
	QLabel *wTable;
	
	Seat *wseats[10];
	
	QLabel *lblPots;
	WPicture *cc[5];
	QWidget *wCC;
	

	
private:
	//! \brief Game ID
	const int	m_nGid;
	//! \brief Table ID
	const int	m_nTid;
	//! \brief Tablechat
	ChatBox			*m_pChat;
	//! \brief Container for Table Actions
//	QStackedLayout	*m_stLayActions;
	//! \brief Layout for Table Actions
	QLabel			*m_LayoutActions;
	//! \brief Dealer Button
	DealerButton	*m_pDealerButton;
	//! \brief Seats
//	std::vector<Seat*>	m_Seats;		// TODO: über scene ansprechen
};

#endif /* _WTABLE_H */
