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

#include "Debug.h"
#include "Card.hpp"
#include "Deck.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "GameLogic.hpp"

#if 0
#define HW_RANDOM	"/dev/urandom"
#endif

using namespace std;

/* Combination probabilities:
 * 	http://de.wikipedia.org/wiki/Texas_Holdem#Wahrscheinlichkeiten
 *
 * Start-Hand vs. Random-Hand (Heads-Up) probabilities:
 * 	http://www.thema-poker.com/wahrscheinlichkeiten/starthaende
 */
 
int main(int argc, char **argv)
{
	printf("Poker Hand-Simulator\n");
	
	unsigned long tests;
	
	if (argc < 2)
		tests = 10000000;
	else
		tests = atoi(argv[1]);
	
#ifdef HW_RANDOM
	FILE *fp = fopen(HW_RANDOM, "r");
#else
	// init PRNG
	srand((unsigned) time(NULL));
#endif /* HW_RANDOM */
	
	struct {
		const char *str;
		const double probab;
	} rankings[] = {
		{ "High Card",		.17411920	},
		{ "One Pair",		.438322546	},
		{ "Two Pair",		.23495536	},
		{ "Three Of A Kind", 	.04829870	},
		{ "Straight",		.0461938	},
		{ "Flush",		.0303255	},
		{ "Full House",		.02596102	},
		{ "Four Of A Kind",	.00168067	},
		{ "Straight Flush",	.00027851	},
		{ "Royal Flush",	.00003232	}
	};
	
	
	printf("Iterations: %ld\n", tests);
	
	if (true)
	{
		// all combinations + RoyalFlush
		const int strengths = (HandStrength::StraightFlush - HandStrength::HighCard +1) +1;
		long int count[strengths];
		int last_progress = 0;
		
		for (int i=0; i < strengths; i++)
			count[i] = 0;
		
		printf(".");
		
		for (unsigned long i=0; i < tests; i++)
		{
#ifdef HW_RANDOM
			unsigned long seed;
			size_t read_elements;
			read_elements = fread(&seed, sizeof(seed), 1, fp);
			
			if (read_elements*sizeof(seed) != sizeof(seed))
			{
				fprintf(stderr, "Error reading from random device (bytes read: %d; bytes expected: %d).\n",
					(int)read_elements, (int)sizeof(seed));
				return -1;
			}
			
			srand(seed);
#endif /* HW_RANDOM */

			// progress
			if (!(i % 1000))
			{
				int progress = 80 * i / tests;
				if (progress > last_progress)
				{
					printf(".");
					fflush(stdout);
					last_progress = progress;
				}
			}
			
			Deck *d = new Deck();
			
			d->fill();
			d->shuffle();
			
			
			HoleCards *h = new HoleCards();
			
			Card c1, c2;
			d->pop(c1);
			d->pop(c2);
			h->setCards(c1, c2);
			
			
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
			
			
			HandStrength strength;
			GameLogic::getStrength(h, cc, &strength);
			
			vector<Card> rank;
			strength.copyRankCards(&rank);
			
			// handle RoyalFlush as special case
			if (strength.getRanking() == HandStrength::StraightFlush && rank.front().getFace() == Card::Ace)
				count[strengths-1]++;
			else
				count[strength.getRanking() - HandStrength::HighCard]++;
			
			delete cc;
			delete h;
			delete d;
		}
		
		printf("\n");
		
		for (int i=0; i < strengths; i++)
			printf("%.8lf (%+7.8lf = %+7.6lf%%) - %s\n",
				(double)count[i] / (double) tests,
				(double)count[i] / (double) tests - rankings[i].probab,
				((double)count[i] / (double) tests - rankings[i].probab)*100.0,
				rankings[i].str);
	}
	
	printf("--------------------------------------------------------------------------------\n");
	
	
	
	if (false)
	{
		long int wincount = 0, losecount = 0, splitcount = 0;
		
		HoleCards *h1 = new HoleCards();
		Card card1(Card::Seven,  Card::Clubs);
		Card card2(Card::Seven,  Card::Hearts);
		h1->setCards(card1, card2);
		
		
		for (unsigned long i=0; i < tests; i++)
		{
			Deck *d = new Deck();
			
			d->fill();
			
			// remove test-holecards
			d->debugRemoveCard(card1);
			d->debugRemoveCard(card2);
			
			d->shuffle();
			
			
			HoleCards *h2 = new HoleCards();
			
			Card c1, c2;
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
			GameLogic::getStrength(h2, cc, &strength2);
			
			if (strength1 > strength2)
				wincount++;
			else if (strength1 == strength2)
				splitcount++;
			else if (strength1 < strength2)
				losecount++;
			else { printf("Outch, bug in strength-operator.\n"); return 0; }
			
			delete cc;
			delete h2;
			delete d;
		}
		
		printf("%s ", card1.getName());
		printf("%s - ", card2.getName());
		
		printf("win %4.2lf%%, lose %4.2lf%%, split %4.2lf%%\n",
			100 * ((double) wincount / (double) tests),
			100 * ((double) losecount / (double) tests),
			100 * ((double) splitcount / (double) tests));
	}
	
#ifdef HW_RANDOM
	fclose(fp);
#endif /* HW_RANDOM */
	
	return 0;
}
