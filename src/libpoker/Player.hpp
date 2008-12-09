#ifndef _PLAYER_H
#define _PLAYER_H

#include "HoleCards.hpp"

class Player
{
friend class GameController;

public:
	typedef enum {
		ResetAction,
		Check,
		Fold,
		Call,
		Bet,
		Raise,
		Allin,
		Show
	} PlayerAction;
	
	typedef struct {
		bool valid;
		PlayerAction action;
		float amount;
	} SchedAction;
	
	Player();
	
private:
	int client_id;
	
	float chipstack;
	
	HoleCards holecards;
	
	SchedAction next_action;
};


#endif /* _PLAYER_H */
