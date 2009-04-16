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


#ifndef _HOLDING_NUTS_TIMEOUT_H
#define _HOLDING_NUTS_TIMEOUT_H

#include <QGraphicsItem>
#include <QTimeLine>
#include <QGraphicsItemAnimation>

class TimeOut : public QObject, public QGraphicsItem
{
	Q_OBJECT

public:
	TimeOut();

	QRectF boundingRect() const;

	void paint(
		QPainter* painter,
		const QStyleOptionGraphicsItem* option,
		QWidget* widget);

	void start(int seat, int sec_timeout);
	void stop();
	
Q_SIGNALS:
	//! \brief signal emitted when the time is up
	//! \param seatnumber
	void timeup(int);
	
	void quarterElapsed(int);

	void halfElapsed(int);

	void threeQuarterElapsed(int);

private Q_SLOTS:
	void update(int frame);

private:
	const QImage			m_Image;
	//! \brief Timeline Dealerbutton Animation
	QTimeLine				m_tl;
	//! \brief Framenumber
	int						m_nFrame;
	//! \brief Seat ID
	int						m_nSeat;
	
	bool 					m_bQuarterAlreadyEmitted;
	bool 					m_bHalfAlreadyEmitted;
	bool 					m_bThreeQuarterAlreadyEmitted;
};

#endif /* _HOLDING_NUTS_TIMEOUT_H */
