#include <algorithm>

#include "GameDebug.hpp"
#include "Deck.hpp"

using namespace std;

Deck::Deck()
{

}

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

int Deck::count()
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
