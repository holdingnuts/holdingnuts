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


#ifndef _GAMELOGIC_H
#define _GAMELOGIC_H

#include <vector>

#include "Card.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"

class HandStrength
{
friend class GameLogic;

public:
	typedef enum {
		HighCard=0,
		OnePair,
		TwoPair,
		ThreeOfAKind,
		Straight,
		Flush,
		FullHouse,
		FourOfAKind,
		StraightFlush
	} Ranking;
	
	Ranking getRanking() const { return ranking; };
	static const char* getRankingName(Ranking r);
	
	void copyRankCards(std::vector<Card> *v) const { v->insert(v->end(), rank.begin(), rank.end()); };
	void copyKickerCards(std::vector<Card> *v) const { v->insert(v->end(), kicker.begin(), kicker.end()); };
	
	void setId(int rid) { id = rid; };
	int getId() const { return id; };
	
	bool operator < (const HandStrength &c) const;
	bool operator > (const HandStrength &c) const;
	bool operator == (const HandStrength &c) const;
	
private:
	Ranking ranking;
	std::vector<Card> rank;
	std::vector<Card> kicker;
	
	int id;  // identifier; can be used for associating player
};

class GameLogic
{
public:
	GameLogic();
	
	static bool getStrength(std::vector<Card> *allcards, HandStrength *strength);
	static bool getStrength(const HoleCards *hole, const CommunityCards *community, HandStrength *strength);
	
	static bool isTwoPair(std::vector<Card> *allcards, std::vector<Card> *rank, std::vector<Card> *kicker);
	static bool isStraight(std::vector<Card> *allcards, const int suit, std::vector<Card> *rank);
	static bool isFlush(std::vector<Card> *allcards, std::vector<Card> *rank);
	static bool isXOfAKind(std::vector<Card> *allcards, const unsigned int n, std::vector<Card> *rank, std::vector<Card> *kicker);
	static bool isFullHouse(std::vector<Card> *allcards, std::vector<Card> *rank);
	
	static bool getWinList(std::vector<HandStrength> &hands, std::vector< std::vector<HandStrength> > &winlist);
};


#endif /* _GAMELOGIC_H */
