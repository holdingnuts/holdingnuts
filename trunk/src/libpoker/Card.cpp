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
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


#include "Card.hpp"


static const char face_symbols[] = {
	'2', '3', '4', '5', '6', '7', '8', '9',
	'T', 'J', 'Q', 'K', 'A'
};

static const char suit_symbols[] = {
	'c', 'd', 'h', 's'
};


Card::Card()
{
	face = Card::FirstFace;
	suit = Card::FirstSuit;
}

Card::Card(Face f, Suit s)
{
	face = f;
	suit = s;
}

Card::Card(const char *str)
{
	face = convertFaceSymbol(str[0]);
	suit = convertSuitSymbol(str[1]);
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
	return face_symbols[face - Card::FirstFace];
}

char Card::getSuitSymbol() const
{
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

Card::Face Card::convertFaceSymbol(char fsym)
{
	for (unsigned int i=Card::FirstFace; i <= Card::LastFace; i++)
		if (fsym == face_symbols[i - Card::FirstFace])
			return (Card::Face)i;
		
	return Card::FirstFace;
}

Card::Suit Card::convertSuitSymbol(char ssym)
{
	for (unsigned int i=Card::FirstSuit; i <= Card::LastSuit; i++)
		if (ssym == suit_symbols[i - Card::FirstSuit])
			return (Card::Suit)i;
		
	return Card::FirstSuit;
}
