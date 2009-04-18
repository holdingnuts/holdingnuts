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


#include <algorithm>

#include "GameDebug.hpp"
#include "Deck.hpp"

using namespace std;


void Deck::fill()
{
	cards.clear();
	
	for (int f=Card::FirstFace; f <= Card::LastFace; f++)
		for (int s=Card::FirstSuit; s <= Card::LastSuit; s++)
		{
			Card c((Card::Face)f, (Card::Suit)s);
			push(c);
		}
}

void Deck::empty()
{
	cards.clear();
}

int Deck::count() const
{
	return cards.size();
}

bool Deck::push(Card card)
{
	cards.push_back(card);
	return true;
}

bool Deck::pop(Card &card)
{
	if (!count())
		return false;
	
	card = cards.back();
	cards.pop_back();
	return true;
}

bool Deck::shuffle()
{
	random_shuffle(cards.begin(), cards.end());
	return true;
}


void Deck::debug()
{
	print_cards("Deck", &cards);
}

void Deck::debugRemoveCard(Card card)
{
	for (vector<Card>::iterator e = cards.begin(); e != cards.end(); e++)
	{
		if (e->getFace() == card.getFace() && e->getSuit() == card.getSuit()) {
			cards.erase(e);
			break;
		}
	}
}

void Deck::debugPushCards(const vector<Card> *cardsvec)
{
	for (vector<Card>::const_iterator e = cardsvec->begin(); e != cardsvec->end(); e++)
		push(*e);
}
