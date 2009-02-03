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


#ifndef _TABLE_H
#define _TABLE_H

#include "Deck.hpp"
#include "CommunityCards.hpp"
#include "Player.hpp"
#include "GameLogic.hpp"

class Table
{
friend class GameController;

public:
	typedef enum {
		GameStart,
		ElectDealer,
		NewRound,
		Blinds,
		Betting,
		AskShow,
		AllFolded,
		Showdown,
		EndRound,
		Delay
	} State;
	
	typedef enum {
		Preflop,
		Flop,
		Turn,
		River
	} BettingRound;
	
	typedef struct {
		bool occupied;
		unsigned int seat_no;
		Player *player;
		float bet;
		bool in_round;   // is player involved in current hand?
		bool showcards;  // does the player want to show cards?
	} Seat;
	
	typedef struct {
		float amount;
		std::vector<Player*> players;
		bool final;
	} Pot;
	
	Table();
	
	bool setTableId(int tid) { table_id = tid; return true; };
	int getTableId() { return table_id; };
	
	int getNextPlayer(unsigned int pos);
	int getNextActivePlayer(unsigned int pos);
	unsigned int countPlayers();
	unsigned int countActivePlayers();
	bool isAllin();
	void resetLastPlayerActions();
	
	void collectBets();
	bool isPlayerInvolvedInPot(Pot *pot, Player *p);
	unsigned int getInvolvedInPotCount(Pot *pot, std::vector<HandStrength> &wl);
	
	void scheduleState(State sched_state, unsigned int delay_sec);
	
	void tick();
	
private:
	int table_id;
	
	Deck deck;
	CommunityCards communitycards;
	
	State state;
	
	// Delay state
	State scheduled_state;
	time_t delay_start;
	unsigned int delay;
	
	bool nomoreaction;
	BettingRound betround;
	
	Seat seats[10];
	unsigned int dealer, sb, bb;
	unsigned int cur_player;
	unsigned int last_bet_player;
	
	float bet_amount;
	float last_bet_amount;
	std::vector<Pot> pots;
};


#endif /* _TABLE_H */
