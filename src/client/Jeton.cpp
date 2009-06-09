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


#include "Jeton.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

Jeton::Jeton(const QColor& c, QGraphicsItem* parent)
:	QGraphicsItem(parent),
	m_Color(c)
{ }

QRectF Jeton::boundingRect() const
{
	QRectF rc(0, 0, 40, 30);
	QTransform m = this->transform();
	
	return m.mapRect(rc);
}

void Jeton::paint(
	QPainter* painter,
	const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{
	painter->save();
		painter->setBrush(m_Color);
		painter->drawRoundedRect(0, -15, 40, 30, 8, 8);
	painter->restore();	
}
