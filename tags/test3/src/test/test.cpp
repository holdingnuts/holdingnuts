#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <vector>
#include <algorithm>
#include <functional>

#include "Platform.h"
#include "Debug.h"

#include "Card.hpp"
#include "Deck.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "GameLogic.hpp"

using namespace std;

int main(void)
{
	printf("Poker-test\n");
	printf("Running on ");
	
#if defined(PLATFORM_UNIX)
	printf("UNIX\n");
#elif defined(PLATFORM_WINDOWS)
	printf("WINDOWS\n");
#else
	printf("UNKNOWN\n");
#endif
	
	// init PRNG
	srand((unsigned) time(NULL));
	
	
	/*
	Card *c = new Card(Card::Ace, Card::Spades);
	printf("Card face=%c suit=%c => %s\n",
		c->getFaceSymbol(),
		c->getSuitSymbol(),
		c->getName()
		);
	delete c;
	*/
	
	Deck *d = new Deck();
	
	
	d->fill();
	d->debug();
	
	d->shuffle();
	d->debug();
	
	
	printf("--------------\n");
	
	
	Card cd1(Card::King, Card::Spades);
	Card cd2(Card::Jack, Card::Hearts);
	
	if (cd1 < cd2)
		printf("c1 < c2\n");
	else if (cd1 > cd2)
		printf("c1 > c2\n");
	else
		printf("Equal\n");
	
/*
0 = RANDOM
1 = Flush
2 = FourOfAKind
3 = Straight AKQJT
4 = Straight A2345
5 = StraightFlush
6 = ThreeOfAKind
7 = FullHouse
8 = TwoPair
9 = OnePair
*/
#define TESTCARDS 10
	
	HoleCards *h1 = new HoleCards();
	HoleCards *h2 = new HoleCards();
	
	Card c1, c2;
	d->pop(c1);
	d->pop(c2);
	
#if (TESTCARDS == 1)
	h1->setCards(
		Card(Card::King, Card::Spades),
		Card(Card::Ten,  Card::Spades)
	);
#elif (TESTCARDS == 2 || TESTCARDS == 6 || TESTCARDS == 7 || TESTCARDS == 9)
	h1->setCards(
		Card(Card::King, Card::Spades),
		Card(Card::King, Card::Hearts)
	);
#elif (TESTCARDS == 3 || TESTCARDS == 5)
	h1->setCards(
		Card(Card::King, Card::Hearts),
		Card(Card::Queen, Card::Hearts)
	);
#elif (TESTCARDS == 4 || TESTCARDS == 8)
	h1->setCards(
		Card(Card::Three, Card::Spades),
		Card(Card::Four, Card::Hearts)
	);
#elif (TESTCARDS == 10)
	h1->setCards(
		Card(Card::King, Card::Spades),
		Card(Card::Ace, Card::Hearts)
	);
#else
	h1->setCards(c1, c2);
#endif
	h1->debug();
	
	d->pop(c1);
	d->pop(c2);
	//h2->setCards(c1, c2);
	
	h2->setCards(
		Card(Card::Three, Card::Diamonds),
		Card(Card::King, Card::Diamonds)
	);
	h2->debug();
	
	
	CommunityCards *cc = new CommunityCards();
	cc->debug();
	
	Card f1, f2, f3, t, r;
	d->pop(f1);
	d->pop(f2);
	d->pop(f3);
	
#if (TESTCARDS == 1 || TESTCARDS == 9)
	cc->setFlop(
		Card(Card::Four, Card::Spades),
		Card(Card::Five, Card::Spades),
		Card(Card::Six, Card::Spades)
	);
#elif (TESTCARDS == 2 || TESTCARDS == 6 || TESTCARDS == 7)
	cc->setFlop(
		Card(Card::King, Card::Clubs),
		Card(Card::Five, Card::Spades),
		Card(Card::Six, Card::Spades)
	);
#elif (TESTCARDS == 3 || TESTCARDS == 5)
	cc->setFlop(
		Card(Card::Ten, Card::Hearts),
		Card(Card::Jack, Card::Spades),
		Card(Card::Queen, Card::Spades)
	);
#elif (TESTCARDS == 4)
	cc->setFlop(
		Card(Card::Five, Card::Clubs),
		Card(Card::Two, Card::Spades),
		Card(Card::Queen, Card::Spades)
	);
#elif (TESTCARDS == 8)
	cc->setFlop(
		Card(Card::Three, Card::Clubs),
		Card(Card::Four, Card::Diamonds),
		Card(Card::Queen, Card::Spades)
	);
#elif (TESTCARDS == 10)
	cc->setFlop(
		Card(Card::Two, Card::Clubs),
		Card(Card::Four, Card::Diamonds),
		Card(Card::Queen, Card::Spades)
	);
#else
	cc->setFlop(f1, f2, f3);
#endif
	cc->debug();
	
	d->pop(t);
	
#if (TESTCARDS == 10)
	cc->setTurn(Card(Card::Eight, Card::Hearts));
#elif (TESTCARDS != 0)
	cc->setTurn(Card(Card::Ace, Card::Hearts));
#else
	cc->setTurn(t);
#endif
	cc->debug();
	
	d->pop(r);

#if (TESTCARDS == 1 || TESTCARDS == 3 || TESTCARDS == 4 || TESTCARDS == 6)
	cc->setRiver(Card(Card::Eight, Card::Spades));
#elif (TESTCARDS == 2 || TESTCARDS == 10)
	cc->setRiver(Card(Card::King, Card::Diamonds));
#elif (TESTCARDS == 5 || TESTCARDS == 9)
	cc->setRiver(Card(Card::Jack, Card::Hearts));
#elif (TESTCARDS == 7 || TESTCARDS == 8)
	cc->setRiver(Card(Card::Six, Card::Diamonds));
#else
	cc->setRiver(r);
#endif
	cc->debug();
	
	d->debug();
	
	printf("-----------\n");
	
	/*
	vector<Card> allcards;
	
	h1->copyCards(&allcards);
	cc->copyCards(&allcards);
	
	sort(allcards.begin(), allcards.end());
	
	printf("AllCards: ! ");
	
	for (vector<Card>::iterator e = allcards.begin(); e != allcards.end(); e++)
		printf("%s ", e->getName());
	
	printf("!\n");
	*/
	
	/*
	GameLogic::Strength strength;
	std::vector<Card> rank, kicker;
	
	strength = GameLogic::getStrength(h1, cc, &rank, &kicker);
	printf("Strength: %s\n", GameLogic::getStrengthName(strength));
	*/
	
	HandStrength strength1, strength2;
	GameLogic::getStrength(h1, cc, &strength1);
	printf("Strength1: %s\n----------------\n", HandStrength::getRankingName(strength1.getRanking()));
	GameLogic::getStrength(h2, cc, &strength2);
	printf("Strength2: %s\n", HandStrength::getRankingName(strength2.getRanking()));
	
	if (strength1 < strength2)
		printf("s1 < s2\n");
	else if (strength1 > strength2)
		printf("s1 > s2\n");
	else
		printf("s1 == s2\n");
	
	printf("#####################\n");
	
	
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
	}
	
	// delete deck
	delete d;
	
	
	printf("#####################\n");
	
#if 0
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
#endif

#if 1
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
		
		vector<HandStrength> wl, w2;
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
		
		winlist.push_back(wl);
		
		int index=0;
		do
		{
			vector<HandStrength> &tw = winlist[index];
			vector<HandStrength> tmp;
			
			sort(tw.begin(), tw.end(), greater<HandStrength>());
			
			for (unsigned int i=tw.size()-1; i > 0; i--)
			{
				if (tw[i] < tw[0])
				{
					tmp.push_back(tw[i]);
					tw.pop_back();
				}
			}
			
			if (!tmp.size())
				break;
			
			winlist.push_back(tmp);
			index++;
			
		} while(true);
		
		for (unsigned int i=0; i < winlist.size(); i++)
		{
			printf("--- Winlist %d---\n", i); fflush(stdout);
			
			vector<HandStrength> &tw = winlist[i];
			
			for (unsigned int j=0; j < tw.size(); j++)
				printf("Rank %d = player %d\n", j, tw[j].getId()); fflush(stdout);
		}
		
	#if 0
		sort(wl.begin(), wl.end(), greater<HandStrength>());
		
		printf("--- Ranking ---\n"); fflush(stdout);
		for (unsigned int i=0; i < wl.size(); i++)
			printf("Rank %d = player %d\n", i, wl[i].id); fflush(stdout);
		
		
		//for (vector<HandStrength>::reverse_iterator e = wl->end(); e != wl->begin(); e++)
		for (unsigned int i=wl.size()-1; i > 0; i--)
		{
			if (wl[i] < wl[0])
			{
				w2.push_back(wl[i]);
				wl.pop_back();
			}
		}
		
		printf("--- wl ---\n"); fflush(stdout);
		for (unsigned int i=0; i < wl.size(); i++)
			printf("Rank %d = player %d\n", i, wl[i].id); fflush(stdout);
		
		printf("--- w2 ---\n"); fflush(stdout);
		for (unsigned int i=0; i < w2.size(); i++)
			printf("Rank %d = player %d\n", i, w2[i].id); fflush(stdout);
	#endif
	}
#endif
	
	return 0;
}
