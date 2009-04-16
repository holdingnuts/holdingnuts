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


#ifndef _HOLDING_NUTS_SEAT_IMAGES_H
#define _HOLDING_NUTS_SEAT_IMAGES_H

#include <QImage>

class SeatImages
{
public:
	static const SeatImages& Instance();

	//! \brief Background Image
	const QImage 		imgBack;
	//! \brief Background Image for current player
	const QImage 		imgBackCurrent;
	//! \brief Background Image for sitout player
	const QImage 		imgBackSitout;
	//! \brief Action Image None
	const QImage		imgActNone;
	//! \brief Action Image Bet
	const QImage		imgActBet;
	//! \brief Action Image Call
	const QImage		imgActCall;
	//! \brief Action Image Check
	const QImage		imgActCheck;
	//! \brief Action Image Fold
	const QImage		imgActFold;
	//! \brief Action Image Muck
	const QImage		imgActMuck;
	//! \brief Action Image Raise
	const QImage		imgActRaise;
	//! \brief Action Image Show
	const QImage		imgActShow;
	//! \brief Action Image Allin
	const QImage		imgActAllin;
	
	//! \brief Status Image Win
	const QImage		imgStatusWin;	

private:
	SeatImages();

	SeatImages(const SeatImages& src);
	SeatImages& operator = (const SeatImages& src);
};

#endif /* _HOLDING_NUTS_SEAT_IMAGES_H */
