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


#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <vector>
#include <algorithm>
#include <functional>

#include "Platform.h"
#include "Logger.h"
#include "Debug.h"

#include "Card.hpp"
#include "Deck.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "GameLogic.hpp"


using namespace std;

int test_card1()
{
	Card *c = new Card(Card::Ace, Card::Spades);
	printf("Card face=%c suit=%c => %s\n",
		c->getFaceSymbol(),
		c->getSuitSymbol(),
		c->getName()
		);
	delete c;
	
	return 0;
}

int test_card2()
{
	Card cd1(Card::King, Card::Spades);
	Card cd2(Card::Jack, Card::Hearts);
	
	if (cd1 < cd2)
		printf("c1 < c2\n");
	else if (cd1 > cd2)
		printf("c1 > c2\n");
	else
		printf("Equal\n");
	
	return 0;
}

int test_card3()
{
	Card c("Jh");
	
	printf("Card face=%c suit=%c => %s\n",
		c.getFaceSymbol(),
		c.getSuitSymbol(),
		c.getName()
		);
	
	return 0;
}

int test_deck1()
{
	Deck *d = new Deck();
	
	d->fill();
	d->debug();
	
	d->shuffle();
	d->debug();
	
	delete d;
	
	return 0;
}

int test_handstrength1()
{
	HoleCards *h = new HoleCards();
	CommunityCards *cc = new CommunityCards();
	
	Card::Suit s = Card::Spades;
	h->setCards(
		Card(Card::Six, s),
		Card(Card::Two, s)
	);
	
	cc->setFlop(
		Card(Card::Five, s),
		Card(Card::Four, s),
		Card(Card::Three, s/*Card::Diamonds*/)
	);
	
	cc->setTurn(Card(Card::Two, Card::Clubs));
	cc->setRiver(Card(Card::Three, Card::Diamonds));
	
	HandStrength hs;
	GameLogic::getStrength(h, cc, &hs);
	printf("Strength: %s\n----------------\n", HandStrength::getRankingName(hs.getRanking()));
	
	delete h;
	delete cc;
	
	return 0;
}

int test_handstrength2()
{
	for(;;)
	{
		Deck *d = new Deck();
		
		d->fill();
		d->shuffle();
		
		
		HoleCards *h1 = new HoleCards();
		HoleCards *h2 = new HoleCards();
		
		Card c1, c2;
		d->pop(c1);
		d->pop(c2);
		h1->setCards(c1, c2);
		
		d->pop(c1);
		d->pop(c2);
		h2->setCards(c1, c2);
		
		
		CommunityCards *cc = new CommunityCards();
		
		Card f1, f2, f3, t, r;
		d->pop(f1);
		d->pop(f2);
		d->pop(f3);
		cc->setFlop(f1, f2, f3);
		d->pop(t);
		cc->setTurn(t);
		d->pop(r);
		cc->setRiver(r);
		
		
		HandStrength strength1, strength2;
		GameLogic::getStrength(h1, cc, &strength1);
		printf("---\n");
		GameLogic::getStrength(h2, cc, &strength2);
		
		const char* lge = "";
		if (strength1 > strength2)
			lge = ">";
		else if (strength1 == strength2)
			lge = "==";
		else if (strength1 < strength2)
			lge = "<";
		else { printf("Outch, bug in strength-operator\n"); return 0; }
		
		printf("Strength: %s %s %s\n",
			HandStrength::getRankingName(strength1.getRanking()),
			lge,
			HandStrength::getRankingName(strength2.getRanking())
		);
		printf("----------------------\n");
		
		delete cc;
		delete h1;
		delete h2;
		delete d;
		
		getchar();
	}
	
	return 0;
}

int test_winlist1()
{
	Deck d;
	d.fill();
	d.shuffle();
	
	Card f1, f2, f3, t, r;
	CommunityCards cc;
	
	d.pop(f1); d.pop(f2); d.pop(f3); d.pop(t); d.pop(r);
	cc.setFlop(f1, f2, f3);
	cc.setTurn(t);
	cc.setRiver(r);
	
	const unsigned int players = 4;
	
	HoleCards h[players];
	HandStrength hs[players];
	
	vector<HandStrength> wl;
	vector< vector<HandStrength> > winlist;
	
	for (unsigned int i=0; i < players; i++)
	{
		printf("--- Player %d ---\n", i); fflush(stdout);
		
		Card c1, c2;
		d.pop(c1);
		d.pop(c2);
		h[i].setCards(c1, c2);
		
		GameLogic::getStrength(&(h[i]), &cc, &(hs[i]));
		hs[i].setId(i);
		
		wl.push_back(hs[i]);
	}
	
	
	// determine winlist
	GameLogic::getWinList(wl, winlist);
	
	
	for (unsigned int i=0; i < winlist.size(); i++)
	{
		printf("--- Winlist %d---\n", i); fflush(stdout);
		
		vector<HandStrength> &tw = winlist[i];
		
		for (unsigned int j=0; j < tw.size(); j++)
			printf("Rank %d = player %d\n", j, tw[j].getId()); fflush(stdout);
	}
	
	return 0;
}


int main(void)
{
	printf("Poker-test; running on ");
	
#if defined(PLATFORM_UNIX)
	printf("UNIX\n");
#elif defined(PLATFORM_WINDOWS)
	printf("WINDOWS\n");
#else
	printf("UNKNOWN\n");
#endif
	
	// init PRNG
	srand((unsigned) time(NULL));
	
#if 0
	test_card1();
	test_card2();
	test_card3();
#endif
	
#if 0
	test_deck1();
#endif
	
#if 0
	test_handstrength1();
	test_handstrength2();
#endif

#if 0
	test_winlist1();
#endif

	return 0;
}
