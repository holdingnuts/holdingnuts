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


#ifndef _HOLDING_NUTS_CHIP_H
#define _HOLDING_NUTS_CHIP_H

#include <QGraphicsItem>
#include <QImage>

class Chip : public QObject, public QGraphicsItem
{
	Q_OBJECT
#if QT_VERSION >= 0x040600
	Q_INTERFACES(QGraphicsItem)
#endif

public:
	Chip(const QImage& img, QGraphicsItem* parent);

	QRectF boundingRect() const;

	void paint(
		QPainter* painter,
		const QStyleOptionGraphicsItem* option,
		QWidget* widget);

private:
	//! \brief
	const QImage	*m_pImage;
};

#endif /* _HOLDING_NUTS_CHIP_H */
