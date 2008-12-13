/*
 * Copyright 2008, Dominik Geyer
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
 */


#include "Card.hpp"

Card::Card()
{
	face = Two;
	suit = Clubs;
}

Card::Card(Face f, Suit s)
{
	face = f;
	suit = s;
}

void Card::getValue(Face *f, Suit *s) const
{
	if (f)
		*f = face;
	
	if (s)
		*s = suit;
}

char Card::getFaceSymbol() const
{
	static const char face_symbols[] = {
		'2', '3', '4', '5', '6', '7', '8', '9',
		'T', 'J', 'Q', 'K', 'A'
	};
	
	return face_symbols[face - Card::FirstFace];
}

char Card::getSuitSymbol() const
{
	static const char suit_symbols[] = {
		'c', 'd', 'h', 's'
	};
	
	return suit_symbols[suit - Card::FirstSuit];
}

const char* Card::getName() const
{
	static char card_name[3];
	
	card_name[0] = getFaceSymbol();
	card_name[1] = getSuitSymbol();
	card_name[2] = '\0';
	
	return card_name;
}
