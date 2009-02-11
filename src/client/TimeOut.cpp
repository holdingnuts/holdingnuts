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

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QDebug>

TimeOut::TimeOut()
:	m_Image("gfx/table/timeout.png"),
	m_nFrame(0),
	m_nSeat(0)
{
	m_tl.setFrameRange(0, m_Image.width());
	
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
	this->setZValue(9);

	painter->setRenderHint(QPainter::SmoothPixmapTransform);

	painter->drawImage(
		QRectF(
			0,
			0,
			m_Image.width(),
			m_Image.height()),
		m_Image);

	if (m_nFrame > 0)
		painter->fillRect(0, 0, m_nFrame, m_Image.height(), Qt::black);
}

void TimeOut::start(int seat, int sec_timeout)
{
	if (m_tl.state() == QTimeLine::Running)
		m_tl.stop();
		
	m_tl.setDuration(sec_timeout * 1000);
	m_tl.start();
	
	m_nSeat = seat;
}

void TimeOut::update(int frame)
{
	m_nFrame = frame;
	
	if (frame == m_Image.width())
		emit timeup(m_nSeat);
	
	scene()->update(this->boundingRect());
}
