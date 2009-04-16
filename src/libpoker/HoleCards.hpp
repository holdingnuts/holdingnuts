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


#ifndef _HOLECARDS_H
#define _HOLECARDS_H

#include <vector>

#include "Card.hpp"

class HoleCards
{
public:
	HoleCards();
	
	bool setCards(Card c1, Card c2);
	void clear() { cards.clear(); };
	
	void copyCards(std::vector<Card> *v) const { v->insert(v->end(), cards.begin(), cards.end()); };
	
	void debug();
private:
	std::vector<Card> cards;
};

#endif /* _HOLECARDS_H */
