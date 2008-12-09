#ifndef _GAMEDEBUG_H
#define _GAMEDEBUG_H

#include <vector>

#include "Card.hpp"

#if DEBUG
void print_cards(const char *name, std::vector<Card> *cards);
#else
# define print_cards(args...)
#endif

#endif /* _GAMEDEBUG_H */
