/*
 * Copyright 2008, Dominik Geyer
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
		EndRound
	} State;
	
	typedef enum {
		Preflop,
		Flop,
		Turn,
		River
	} BettingRound;
	
	typedef struct {
		unsigned int seat_no;
		Player *player;
		float bet;
		bool in_round;
		bool showcards;
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
	unsigned int countActivePlayers();
	bool isAllin();
	
	void collectBets();
	bool isPlayerInvolvedInPot(Pot *pot, Player *p);
	unsigned int getInvolvedInPotCount(Pot *pot, std::vector<HandStrength> &wl);
	
	float determineMinimumBet() const;
	
	void tick();
	
private:
	int table_id;
	
	Deck deck;
	CommunityCards communitycards;
	
	State state;
	bool nomoreaction;
	BettingRound betround;
	
	unsigned int blind;
	
	std::vector<Seat> seats;
	unsigned int dealer, sb, bb;
	unsigned int cur_player;
	unsigned int last_bet_player;
	
	float bet_amount;
	float last_bet_amount;
	std::vector<Pot> pots;
};


#endif /* _TABLE_H */
