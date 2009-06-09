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
#include "Jeton.hpp"
#include "ChipStake.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

ChipStake::ChipStake(QGraphicsItem* parent)
:	QGraphicsItem(parent)
{ }

QRectF ChipStake::boundingRect() const
{
	QTransform m = this->transform();
	
	return m.mapRect(childrenBoundingRect());
}

void ChipStake::paint(
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

void ChipStake::setAmount(chips_type amount)
{
	clear();

	unsigned num_red = 0;
	unsigned num_blue = 0;
	unsigned num_green = 0;
	unsigned num_black = 0;
	unsigned num_magenta = 0;
	unsigned num_orange = 0;
	unsigned num_grey = 0;
	unsigned num_yellow = 0;

	// jetons
	amount = calc(amount, 20000, num_yellow);
	amount = calc(amount,  5000, num_grey);
	amount = calc(amount,  1000, num_orange);
	amount = calc(amount,   500, num_magenta);
	// chips
	amount = calc(amount,   100, num_black);
	amount = calc(amount,    25, num_green);
	amount = calc(amount,    10, num_blue);
	amount = calc(amount,     5, num_red);
	// remain is white
	
	qreal posy = 0;
	qreal posz = 0;

	// add jetons
	addJetons(num_magenta, Qt::magenta, posy, posz);
	addJetons(num_orange, QColor(255,165,0), posy, posz);
	addJetons(num_grey, Qt::lightGray, posy, posz);
	addJetons(num_yellow, Qt::yellow, posy, posz);
	
	if (posy < 0)
		posy -= 35;
	
	addChips(amount, Qt::white, posy, posz);
	addChips(num_red, Qt::red, posy, posz);
	addChips(num_blue, Qt::blue, posy, posz);
	addChips(num_green, Qt::green, posy, posz);
	addChips(num_black, Qt::darkGray, posy, posz);
}

void ChipStake::clear()
{
	QList<QGraphicsItem*> lst = childItems();
	QList<QGraphicsItem*>::iterator it;
	
	for (it = lst.begin(); it != lst.end(); ++it)
		scene()->removeItem(*it);
}

void ChipStake::addChips(unsigned num, const QColor& c, qreal& y, qreal& z)
{
	for (unsigned i = 0; i < num; ++i)
	{
		Chip* p = new Chip(c, this);
		
		y -= 8;
		z += 1;
		
		p->setPos(5, y);
		p->setZValue(z);
	}
}

void ChipStake::addJetons(unsigned num, const QColor& c, qreal& y, qreal& z)
{
	for (unsigned i = 0; i < num; ++i)
	{
		Jeton* p = new Jeton(c, this);
		
		y -= 8;
		z += 1;
		
		p->setPos(0, y);
		p->setZValue(z);
	}
}

