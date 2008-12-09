#ifndef _DECK_H
#define _DECK_H

#include <vector>

#include "Card.hpp"

class Deck
{
public:
	Deck();
	
	void fill();
	void empty();
	int count();
	
	bool push(Card card);
	bool pop(Card &card);
	bool shuffle();
	
	void debugRemoveCard(Card card);
	void debug();
	
private:
	std::vector<Card> cards;
};

#endif /* _DECK_H */
