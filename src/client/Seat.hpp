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
#include <QTimeLine>

#include "Player.hpp"

class Seat : public QObject, public QGraphicsItem
{
Q_OBJECT

public:
	Seat(unsigned int id, QWidget *parent);

	void setName(const QString& name);
	void setStake(qreal amount);
	void setCurrent(bool cur, int sec_timeout);
	void setSitout(bool sitout);
	void setMySeat(bool bMyseat);
	void setAction(Player::PlayerAction action, qreal amount = 0.0);
	void setCards(const char *c1, const char *c2);
	void setValid(bool valid);
	void showBigCards(bool bShow) { m_bBigCards = bShow; };
	void showSmallCards(bool bShow) { m_bSmallCards = bShow; };

	virtual QRectF boundingRect() const;
	virtual QRectF boundingRectSeat() const;
	
	virtual void paint(
		QPainter* painter,
		const QStyleOptionGraphicsItem* option,
		QWidget* widget);

private:
	void calcSCardPos(qreal& x, qreal& y) const;
	
public:
	// cards size
	static const qreal sx_card;
	static const qreal sy_card;
	// mini cards size
	static const qreal sx_mini_card;
	static const qreal sy_mini_card;
	
private:
	//! \brief Seat ID (clockwise)
	const unsigned int		m_nID;
	//! \brief Playername on the Seat
	QString					m_strName;
	//! \brief
	QString					m_strStake;
	//! \brief
	QString					m_strAmount;
	//! \brief 
	bool					m_bValid;
	//! \brief				
	bool					m_bMySeat;
	//! \brief
	bool					m_bCurrent;
	//! \brief
	bool					m_bSitout;
	//! \brief Pointer to current Action Image
	const QImage			*m_pCurrentActionImg;
	//! \brief first Card
	QImage					m_FirstCard;
	//! \brief second Card
	QImage					m_SecondCard;
	//! \brief
	QTimeLine				m_timeLine;
	//! \brief display big-cards
	bool					m_bSmallCards;
	//! \brief display small-cards
	bool					m_bBigCards;
};

#endif /* _HOLDING_NUTS_SEAT_H */
