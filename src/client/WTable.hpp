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


#ifndef _WTABLE_H
#define _WTABLE_H

#include <vector>

#include <QApplication>
#include <QWidget>
#include <QFont>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QButtonGroup>
#include <QSlider>

#include "Card.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "GameLogic.hpp"
#include "Player.hpp"


typedef struct {
	bool valid;
	int client_id;
	float bet;
	float stake;
	bool in_round;
	//HoleCards holecards;
} seatinfo;

typedef struct {
	int state;
	int betting_round;
	unsigned int s_dealer;
	unsigned int s_sb;
	unsigned int s_bb;
	unsigned int s_cur;
	std::vector<float> pots;
	CommunityCards communitycards;
	seatinfo seats[10];
} table_snapshot;


class WPicture : public QLabel
{
Q_OBJECT

public:
	WPicture(const char *filename, QWidget *parent = 0);
	void loadImage(const char *filename);
private:
	//int heightForWidth ( int w );

};


class WSeat : public QWidget
{
Q_OBJECT

public:
	WSeat(unsigned int id, QWidget *parent = 0);

	void setName(QString name);
	void setStake(float amount);
	void setAction(Player::PlayerAction action, float amount);
	void setCurrent(bool cur);
	void setCards(const char *c1, const char *c2);
	void setValid(bool valid);
	
private slots:
	

private:
	QLabel *lblCaption;
	QLabel *lblStake;
	WPicture *card1, *card2;
	QLabel *lblAction;
};


class WTable : public QWidget
{
Q_OBJECT

public:
	WTable(int gid, int tid, QWidget *parent = 0);
	void updateView();
	
	

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void actionFold();
	void actionCheckCall();
	void actionBetRaise();

private:
	int gid;
	int tid;
	
	QLineEdit *editAmount;
	
	WSeat *wseats[10];
	
	QLabel *lblPots;
	WPicture *cc[5];
};



#endif /* _WTABLE_H */
