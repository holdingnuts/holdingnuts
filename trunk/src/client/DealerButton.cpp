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

DealerButton::DealerButton()
:	m_Image("gfx/table/dealer_button.png")
{ }

QRectF DealerButton::boundingRect() const
{
	return QRectF(0, 0, m_Image.width(), m_Image.height());
}

void DealerButton::paint(
	QPainter* painter,
	const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{
	this->setZValue(10);

	painter->drawImage(
		QRect(
			0,
			0,
			m_Image.width(),
			m_Image.height()),
		m_Image);
}

// void DealerButton::startAnimationTo() { }

void DealerButton::timerEvent(QTimerEvent* event)
{

}
