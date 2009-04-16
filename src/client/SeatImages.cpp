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


#include "SeatImages.hpp"

const SeatImages& SeatImages::Instance()
{
	static SeatImages instance;

	return instance;
}

SeatImages::SeatImages()
:	imgBack("gfx/table/seat.png"),
	imgBackCurrent("gfx/table/seat_current.png"),
	imgBackSitout("gfx/table/seat_sitout.png"),
	imgActNone("gfx/table/action_none.png"),
	imgActBet("gfx/table/action_bet.png"),
	imgActCall("gfx/table/action_call.png"),
	imgActCheck("gfx/table/action_check.png"),
	imgActFold("gfx/table/action_fold.png"),
	imgActMuck("gfx/table/action_muck.png"),
	imgActRaise("gfx/table/action_raise.png"),
	imgActShow("gfx/table/action_show.png"),
	imgActAllin("gfx/table/action_allin.png"),
	imgStatusWin("gfx/table/action_win.png")
{ }
