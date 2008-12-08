#include "Card.hpp"

Card::Card()
{
	face = Two;
	suit = Clubs;
}

Card::Card(Face f, Suit s)
{
	face = f;
	suit = s;
}

void Card::getValue(Face *f, Suit *s) const
{
	if (f)
		*f = face;
	
	if (s)
		*s = suit;
}

char Card::getFaceSymbol() const
{
	static const char face_symbols[] = {
		'2', '3', '4', '5', '6', '7', '8', '9',
		'T', 'J', 'Q', 'K', 'A'
	};
	
	return face_symbols[face - Card::FirstFace];
}

char Card::getSuitSymbol() const
{
	static const char suit_symbols[] = {
		'c', 'd', 'h', 's'
	};
	
	return suit_symbols[suit - Card::FirstSuit];
}

const char* Card::getName() const
{
	static char card_name[3];
	
	card_name[0] = getFaceSymbol();
	card_name[1] = getSuitSymbol();
	card_name[2] = '\0';
	
	return card_name;
}
