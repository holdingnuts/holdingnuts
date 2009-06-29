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


#include "Chip.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

Chip::Chip(const QImage& img, QGraphicsItem* parent)
:	QGraphicsItem(parent),
	m_pImage(&img)
{ }

QRectF Chip::boundingRect() const
{
	Q_ASSERT_X(m_pImage, Q_FUNC_INFO, "invalid image pointer");

	QRectF rc(0, 0, m_pImage->width(), m_pImage->height());
	QTransform m = this->transform();
	
	return m.mapRect(rc);
}

void Chip::paint(
	QPainter* painter,
	const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{
	Q_ASSERT_X(m_pImage, Q_FUNC_INFO, "invalid image pointer");

	painter->save();
	painter->drawImage(
		QRectF(0, 0, m_pImage->width(), m_pImage->height()),
		*m_pImage);
	painter->restore();
}
