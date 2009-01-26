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
 *     Michael Miller <michael.miller@holdingnuts.net>
 */


#ifndef _HOLDING_NUTS_SEAT_H
#define _HOLDING_NUTS_SEAT_H

#include <QGraphicsItem>
#include <QObject>
#include <QLabel>

#include "Player.hpp"

class WPicture : public QLabel
{
Q_OBJECT

public:
	WPicture(const char *filename, QWidget *parent = 0);
	void loadImage(const char *filename);
private:
	//int heightForWidth ( int w );
};

class Seat : public QObject, public QGraphicsItem
{
Q_OBJECT

public:
	Seat(unsigned int id, QWidget *parent);

	void setName(const QString& name);
	void setStake(float amount);
	void setCurrent(bool cur);
	void setMySeat(bool bMyseat);
	void setAction(Player::PlayerAction action, float amount=0.0f);
	void setCards(const char *c1, const char *c2);
	void setValid(bool valid);
	
	
	void setNewRound();
	
	WPicture *card1, *card2;   // FIXME: :)
	WPicture *scard1, *scard2;
	
	void move(int x, int y);
	int width() const;
	int height() const;
	
	virtual QRectF boundingRect() const;

protected:
	virtual void paint(
		QPainter* painter,
		const QStyleOptionGraphicsItem* option,
		QWidget* widget);
	
private slots:
	

private:
	QLabel *lblCaption;
	QLabel *lblStake;
	QLabel *lblAction;
	
private:
	//! \brief Seat ID (clockwise)
	const unsigned int		m_nID;
	//! \brief Playername on the Seat
	QString					m_strName;
	//! \brief
	QString					m_strAmount;
	//! \brief 
	bool					m_bValid;
	//! \brief				
	bool					m_bMySeat;
	//! \brief
	bool					m_bCurrent;
	//! \brief Pointer to current Action Image
	const QImage			*m_pCurrentActionImg;
	//! \brief first Card
	QImage					m_FirstCard;
	//! \brief second Card
	QImage					m_SecondCard;
};

#endif /* _HOLDING_NUTS_SEAT_H */
