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


#ifndef _HOLDING_NUTS_DEALER_BUTTON_H
#define _HOLDING_NUTS_DEALER_BUTTON_H

#include <QGraphicsItem>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

class DealerButton : public QObject, public QGraphicsItem
{
	Q_OBJECT

public:
	DealerButton();

	QRectF boundingRect() const;

	void paint(
		QPainter* painter,
		const QStyleOptionGraphicsItem* option,
		QWidget* widget);

	//! \brief start animation from current pos to center of the seat
	//! \brief minus distance
	void startAnimation(const QPointF& ptCenterSeat, int distance);

	//! \brief start animation from current pos to target pos
	void startAnimation(const QPointF& ptTraget);

private:
	const QImage	m_Image;
	//! \brief Timeline Dealerbutton Animation
	QTimeLine				m_tlDealerBtn;
	//! \brief Animation Dealerbutton
	QGraphicsItemAnimation	m_animDealerBtn;
};

#endif /* _HOLDING_NUTS_DEALER_BUTTON_H */
