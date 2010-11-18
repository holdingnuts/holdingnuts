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
#include "ConfigParser.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

extern ConfigParser config;

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
{
#ifdef DEBUG	
	if (config.getBool("dbg_bbox"))
	{
		painter->save();
		painter->setPen(Qt::cyan);
		painter->drawRect(this->boundingRect());
		painter->restore();	
	}
#endif
}

unsigned calc(chips_type& amount, chips_type value)
{
	if (amount < value)
		return 0;

	unsigned num = abs(amount / value);
	amount = amount % value;

	return num;
}

void ChipStack::setAmount(chips_type amount)
{
	clear();
	
	// load images
	static const QImage imgJeton1("gfx/table/jeton_1.png");
	static const QImage imgJeton5("gfx/table/jeton_5.png");
	static const QImage imgJeton10("gfx/table/jeton_10.png");
	static const QImage imgJeton25("gfx/table/jeton_25.png");
	static const QImage imgJeton100("gfx/table/jeton_100.png");
	static const QImage imgJeton500("gfx/table/jeton_500.png");
	static const QImage imgJeton1000("gfx/table/jeton_1000.png");
	static const QImage imgJeton5000("gfx/table/jeton_5000.png");
	static const QImage imgJeton25000("gfx/table/jeton_25000.png");

	// display amount as tooltip, too
	setToolTip(QString("%1").arg(amount));

	qreal posx = 0;
	qreal posy = 0;

	// big to small
	addChips(calc(amount, 25000), imgJeton25000, posx, posy);
	addChips(calc(amount, 5000),  imgJeton5000, posx, posy);
	addChips(calc(amount, 1000),  imgJeton1000, posx, posy);
	addChips(calc(amount, 500),   imgJeton500, posx, posy);
	addChips(calc(amount, 100),   imgJeton100, posx, posy);

	posy -= 30;
	posx =  17; // 33*0.5

	addChips(calc(amount, 25), imgJeton25, posx, posy);
	addChips(calc(amount, 10), imgJeton10, posx, posy);
	addChips(calc(amount, 5),  imgJeton5, posx, posy);
	addChips(amount,           imgJeton1, posx, posy); // remain is white
}

void ChipStack::clear()
{
	QList<QGraphicsItem*> lst = childItems();
	QList<QGraphicsItem*>::iterator it;
	
	for (it = lst.begin(); it != lst.end(); ++it)
		scene()->removeItem(*it);
}

void ChipStack::addChips(
	unsigned num,
	const QImage& img,
	qreal& x,
	qreal& y)
{
	const qreal save_y = y;
	qreal z = 5;
	
	for (unsigned i = 0; i < num; ++i)
	{
		Chip* p = new Chip(img, this);
		
		y -= 8;
		z += 1;
		
		p->setPos(x, y);
		p->setZValue(z);
	}
	
	if (num > 0)
		x += 32;

	y = save_y;
}
