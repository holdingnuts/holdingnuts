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


#include "TimeOut.hpp"
#include "ConfigParser.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QDebug>

extern ConfigParser config;

TimeOut::TimeOut()
:	m_Image("gfx/table/timeout.png"),
	m_nFrame(0),
	m_nSeat(0),
	m_bQuarterAlreadyEmitted(false),
	m_bHalfAlreadyEmitted(false),
	m_bThreeQuarterAlreadyEmitted(false)
{
	this->setEnabled(false);
	this->setZValue(10);
	
	m_tl.setFrameRange(0, m_Image.width());
	m_tl.setUpdateInterval(200);

	connect(&m_tl, SIGNAL(frameChanged(int)), this, SLOT(update(int)));
}

QRectF TimeOut::boundingRect() const
{
	QRectF rc(0, 0, m_Image.width(), m_Image.height());
	QTransform m = this->transform();
	
	return m.mapRect(rc);
}

void TimeOut::paint(
	QPainter* painter,
	const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	
	painter->save();

	painter->setClipRegion(
		QRect(m_nFrame, 0, m_Image.width(), m_Image.height()));

	painter->drawImage(
		QRectF(0, 0, m_Image.width(), m_Image.height()),
		m_Image);

#ifdef DEBUG	
	if (config.getBool("dbg_bbox"))
	{
		painter->setPen(Qt::blue);
		painter->drawRect(this->boundingRect());
	}
#endif

	painter->restore();
}

void TimeOut::start(int seat, int sec_timeout)
{
	if (m_tl.state() == QTimeLine::Running)
		this->stop();
	
	m_tl.setDuration(sec_timeout * 1000);
	m_tl.start();

	m_nSeat = seat;
	
	m_bQuarterAlreadyEmitted = false;
	m_bHalfAlreadyEmitted = false;
	m_bThreeQuarterAlreadyEmitted = false;
}

void TimeOut::stop()
{
	m_tl.stop();
	m_tl.setCurrentTime(0);
}

void TimeOut::update(int frame)
{
	m_nFrame = frame;
	
	// pass 1/4
	if (m_nFrame > m_Image.width() / 4)
	{
		if (!m_bQuarterAlreadyEmitted)
		{
			emit quarterElapsed(m_nSeat);
			m_bQuarterAlreadyEmitted = true;
		}
	}
	
	// pass 2/4
	if (m_nFrame > m_Image.width() / 2)
	{
		if (!m_bHalfAlreadyEmitted)
		{
			emit halfElapsed(m_nSeat);
			m_bHalfAlreadyEmitted = true;
		}
	}
	
	// pass 3/4
	if (m_nFrame > m_Image.width() / 4 * 3)
	{
		if (!m_bThreeQuarterAlreadyEmitted)
		{
			emit threeQuarterElapsed(m_nSeat);
			m_bThreeQuarterAlreadyEmitted = true;
		}
	}

	if (m_nFrame == m_Image.width())
		emit timeup(m_nSeat);

	QGraphicsItem::update(this->boundingRect());
}
