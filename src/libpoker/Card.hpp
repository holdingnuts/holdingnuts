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


#ifndef _CARD_H
#define _CARD_H

class Card
{
public:
	typedef enum {
		Two=2,
		Three,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		Ten,
		Jack,
		Queen,
		King,
		Ace,
		
		FirstFace=Two,
		LastFace=Ace
	} Face;
	
	typedef enum {
		Clubs=1,
		Diamonds,
		Hearts,
		Spades,
		
		FirstSuit=Clubs,
		LastSuit=Spades
	} Suit;
	
	Card();
	Card(Face f, Suit s);
	Card(const char *str);
	
	void getValue(Face *f, Suit *s) const;
	Face getFace() const { return face; };
	Suit getSuit() const { return suit; };
	
	char getFaceSymbol() const;
	char getSuitSymbol() const;
	const char* getName() const;
	
	bool operator <  (const Card &c) const { return (getFace() < c.getFace()); };
	bool operator >  (const Card &c) const { return (getFace() > c.getFace()); };
	bool operator == (const Card &c) const { return (getFace() == c.getFace()); };
	
	static Face convertFaceSymbol(char fsym);
	static Suit convertSuitSymbol(char ssym);

private:
	Face face;
	Suit suit;
};


#endif /* _CARD_H */
