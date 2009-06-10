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


#include "DealerButton.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

#include <cmath>

qreal normalize(QPointF& vector)
{
	const qreal len = std::sqrt(
		vector.x() * vector.x() +
		vector.y() * vector.y());

	if(len != 0)
	{
		const qreal InvLen = 1 / len;

		vector *= InvLen;
	}
	return len;
}

DealerButton::DealerButton()
:	m_Image("gfx/table/dealer_button.png")
{
	m_tlDealerBtn.setDuration(2000); // 2 seconds
	m_tlDealerBtn.setFrameRange(0, 100);

	m_animDealerBtn.setItem(this);
	m_animDealerBtn.setTimeLine(&m_tlDealerBtn);
	
	this->setZValue(10);
}

QRectF DealerButton::boundingRect() const
{
	QRectF rc(0, 0, m_Image.width(), m_Image.height());
	QTransform m = this->transform();
	
	return m.mapRect(rc);
}

void DealerButton::paint(
	QPainter* painter,
	const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{
	painter->save();
	painter->setRenderHint(QPainter::SmoothPixmapTransform);
	painter->drawImage(
		QRectF(
			0,
			0,
			m_Image.width(),
			m_Image.height()),
		m_Image);
	painter->restore();	
}

void DealerButton::startAnimation(const QPointF& ptCenterSeat, int distance)
{
	if (m_tlDealerBtn.state() == QTimeLine::Running)
		return;

	const QPointF ptMid = this->scene()->sceneRect().center();
	QPointF vDir = ptCenterSeat - ptMid;
	
	normalize(vDir);

	startAnimation(ptCenterSeat - (vDir * distance));
}

void DealerButton::startAnimation(const QPointF& ptTraget)
{
	if (m_tlDealerBtn.state() == QTimeLine::Running)
		return;

	m_animDealerBtn.clear();
	m_animDealerBtn.setPosAt(0, this->scenePos());
	m_animDealerBtn.setPosAt(1, ptTraget);

	m_tlDealerBtn.start();
}
