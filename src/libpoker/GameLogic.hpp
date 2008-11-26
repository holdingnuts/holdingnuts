#ifndef _GAMELOGIC_H
#define _GAMELOGIC_H

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
	
	void copyRankCards(std::vector<Card> *v) { v->insert(v->end(), rank.begin(), rank.end()); };
	void copyKickerCards(std::vector<Card> *v) { v->insert(v->end(), kicker.begin(), kicker.end()); };
	
	bool operator < (const HandStrength &c) const;
	bool operator > (const HandStrength &c) const;
	bool operator == (const HandStrength &c) const;
	
private:
	Ranking ranking;
	std::vector<Card> rank;
	std::vector<Card> kicker;
};

class GameLogic
{
public:
	GameLogic();
	
	static bool getStrength(HoleCards *hole, CommunityCards *community, HandStrength *strength);
	static bool isTwoPair(std::vector<Card> *allcards, std::vector<Card> *rank, std::vector<Card> *kicker);
	static bool isStraight(std::vector<Card> *allcards, const int suit, std::vector<Card> *rank);
	static bool isFlush(std::vector<Card> *allcards, std::vector<Card> *rank);
	static bool isXOfAKind(std::vector<Card> *allcards, const unsigned int n, std::vector<Card> *rank, std::vector<Card> *kicker);
	static bool isFullHouse(std::vector<Card> *allcards, std::vector<Card> *rank);

private:
	
};


#endif /* _GAMELOGIC_H */
