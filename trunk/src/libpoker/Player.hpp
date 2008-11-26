#ifndef _PLAYER_H
#define _PLAYER_H

#include "HoleCards.hpp"

class Player
{
friend class GameController;

public:
	Player();
	
private:
	int client_id;
	
	HoleCards holecards;
};


#endif /* _PLAYER_H */
