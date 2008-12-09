#ifndef _HOLECARDS_H
#define _HOLECARDS_H

#include <vector>

#include "Card.hpp"

class HoleCards
{
public:
	HoleCards();
	
	bool setCards(Card c1, Card c2);
	
	void copyCards(std::vector<Card> *v) { v->insert(v->end(), cards.begin(), cards.end()); };
	
	void debug();
private:
	std::vector<Card> cards;
};

#endif /* _HOLECARDS_H */
