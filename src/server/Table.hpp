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

#include <ctime>

#include "Deck.hpp"
#include "CommunityCards.hpp"
#include "Player.hpp"
#include "GameLogic.hpp"

class Table
{
friend class GameController;
friend class TestCaseGameController;

public:
	typedef enum {
		GameStart,
		ElectDealer,
		NewRound,
		Blinds,
		Betting,
		BettingEnd,  // pseudo-state
		AskShow,
		AllFolded,
		Showdown,
		EndRound
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
		chips_type bet;
		bool in_round;   // is player involved in current hand?
		bool showcards;  // does the player want to show cards?
	} Seat;
	
	typedef struct {
		chips_type amount;
		std::vector<unsigned int> vseats;
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
	bool isSeatInvolvedInPot(Pot *pot, unsigned int s);
	unsigned int getInvolvedInPotCount(Pot *pot, std::vector<HandStrength> &wl);
	
	void scheduleState(State sched_state, unsigned int delay_sec);
	
	void tick();
	
private:
	int table_id;
	
	Deck deck;
	CommunityCards communitycards;
	
	State state;
	
	// Delay state
	time_t delay_start;
	unsigned int delay;
	
	// player timeout
	time_t timeout_start;
	
	bool nomoreaction;
	BettingRound betround;
	
	Seat seats[10];
	int dealer, sb, bb;
	int cur_player;
	int last_bet_player;
	
	chips_type bet_amount;
	chips_type last_bet_amount;
	std::vector<Pot> pots;
};


#endif /* _TABLE_H */
