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

using namespace std;

/* Combination probabilities:
 * 	http://de.wikipedia.org/wiki/Texas_Holdem#Wahrscheinlichkeiten
 *
 * Start-Hand vs. Random-Hand (Heads-Up) probabilities:
 * 	http://www.thema-poker.com/wahrscheinlichkeiten/starthaende
 */
 
int main(void)
{
	printf("Poker Hand-Simulator\n");
	
	// init PRNG
	srand((unsigned) time(NULL));
	
	struct {
		const char *str;
		const double probab;
	} rankings[] = {
		{ "High Card",		17.411920	},
		{ "One Pair",		43.8322546	},
		{ "Two Pair",		23.495536	},
		{ "Three Of A Kind", 	4.829870	},
		{ "Straight",		4.61938		},
		{ "Flush",		3.03255		},
		{ "Full House",		2.596102	},
		{ "Four Of A Kind",	0.168067	},
		{ "Straight Flush",	0.027851	},
		{ "Royal Flush",	0.003232	}
	};
	
	const long int tests = 1000000;
	
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
		
		for (long int i=0; i < tests; i++)
		{
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
			printf("%.6lf (%7.4lf%%  %+7.4lf%%) - %s\n",
				(double)count[i] / (double) tests,
				100 * ((double)count[i] / (double) tests),
				100 * ((double)count[i] / (double) tests) - rankings[i].probab,
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
		
		
		for (long int i=0; i < tests; i++)
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
	
	return 0;
}
