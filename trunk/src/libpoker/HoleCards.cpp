#include "GameDebug.hpp"
#include "HoleCards.hpp"

using namespace std;

HoleCards::HoleCards()
{

}

bool HoleCards::setCards(Card c1, Card c2)
{
	cards.clear();
	cards.push_back(c1);
	cards.push_back(c2);
	
	return true;
}

void HoleCards::debug()
{
	print_cards("Hole", &cards);
}
