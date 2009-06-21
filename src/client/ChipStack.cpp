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
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


#include "Chip.hpp"
#include "ChipStack.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

ChipStack::ChipStack(QGraphicsItem* parent)
:	QGraphicsItem(parent)
{ }

QRectF ChipStack::boundingRect() const
{
	QTransform m = this->transform();
	
	return m.mapRect(childrenBoundingRect());
}

void ChipStack::paint(
	QPainter* painter,
	const QStyleOptionGraphicsItem* option,
	QWidget* widget)
{ }

chips_type calc(chips_type amount, chips_type value, unsigned& num)
{
	if (amount < value)
	{
		num = 0;
		return amount;
	}
		
	num = abs(amount / value);

	return amount % value;
}

void ChipStack::setAmount(chips_type amount)
{
	clear();

	// display amount as tooltip, too
	setToolTip(QString("%1").arg(amount));


	unsigned num_red = 0;
	unsigned num_blue = 0;
	unsigned num_green = 0;
	unsigned num_black = 0;
	unsigned num_magenta = 0;
	unsigned num_orange = 0;
	unsigned num_grey = 0;
	unsigned num_yellow = 0;

	// chips
	amount = calc(amount, 20000, num_yellow);
	amount = calc(amount,  5000, num_grey);
	amount = calc(amount,  1000, num_orange);
	amount = calc(amount,   500, num_magenta);
	amount = calc(amount,   100, num_black);
	amount = calc(amount,    25, num_green);
	amount = calc(amount,    10, num_blue);
	amount = calc(amount,     5, num_red);
	// remaining chips are white
	
	qreal posx = 5;

	// add chips
	addChips(amount, Qt::white, posx);
	addChips(num_red, Qt::red, posx);
	addChips(num_blue, Qt::blue, posx);
	addChips(num_green, Qt::green, posx);
	addChips(num_black, Qt::darkGray, posx);
	addChips(num_magenta, Qt::magenta, posx);
	addChips(num_orange, QColor(255,165,0), posx);
	addChips(num_grey, Qt::lightGray, posx);
	addChips(num_yellow, Qt::yellow, posx);
}

void ChipStack::clear()
{
	QList<QGraphicsItem*> lst = childItems();
	QList<QGraphicsItem*>::iterator it;
	
	for (it = lst.begin(); it != lst.end(); ++it)
		scene()->removeItem(*it);
}

void ChipStack::addChips(unsigned num, const QColor& c, qreal& x)
{
	qreal y = 0;
	qreal z = 0;
	
	for (unsigned i = 0; i < num; ++i)
	{
		Chip* p = new Chip(c, this);
		
		y -= 8;
		z += 1;
		
		p->setPos(x, y);
		p->setZValue(z);
	}
	
	if (num)
		x += 30;
}

