/*
 * Copyright 2008, 2009, Dominik Geyer
 *
 * This file is part of HoldingNuts.
 *
 * HoldingNuts is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoldingNuts is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoldingNuts.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


#ifndef _PLAYER_H
#define _PLAYER_H

#include "HoleCards.hpp"


typedef unsigned int chips_type;


class Player
{
friend class GameController;
friend class TestCaseGameController;

public:
	typedef enum {
		None,
		ResetAction,
		
		Check,
		Fold,
		Call,
		Bet,
		Raise,
		Allin,
		
		Show,
		Muck,
		
		Sitout,
		Back
	} PlayerAction;
	
	typedef struct {
		bool valid;
		PlayerAction action;
		chips_type amount;
	} SchedAction;
	
	Player();
	
	chips_type getStake() const { return stake; };
	int getClientId() const { return client_id; };
	
	void resetLastAction() { last_action = Player::None; }
	
private:
	int client_id;
	
	chips_type stake;		// currrent stake
	chips_type stake_before;	// stake before new hand
	
	HoleCards holecards;
	
	SchedAction next_action;
	PlayerAction last_action;
	
	bool sitout;     // is player sitting out?
};


#endif /* _PLAYER_H */
