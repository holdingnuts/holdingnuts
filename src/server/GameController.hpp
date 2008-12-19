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


#ifndef _GAMECONTROLLER_H
#define _GAMECONTROLLER_H

#include <vector>
#include <ctime>

#include "Card.hpp"
#include "Deck.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "Table.hpp"
#include "Player.hpp"
#include "GameLogic.hpp"

// only variant "Texas Hold'em" supported yet

class GameController
{
public:
	typedef enum {
		RingGame,   // Cash game
		FreezeOut,  // Tournament
		SNG         // Sit'n'Go
	} GameType;
	
	GameController();
	
	bool setGameId(int gid) { game_id = gid; return true; };
	int getGameId() { return game_id; };
	
	GameType getGameType() { return type; };
	
	bool setPlayerMax(unsigned int max);
	unsigned int getPlayerMax() { return max_players; };
	unsigned int getPlayerCount() { return players.size(); };
	
	bool addPlayer(int client_id);
	bool removePlayer(int client_id);
	Player* findPlayer(int cid);
	
	void chat(int tid, const char* msg);
	void chat(int cid, int tid, const char* msg);
	void snap(int tid, int sid, const char* msg);
	void snap(int cid, int tid, int sid, const char* msg);
	
	bool setPlayerAction(int cid, Player::PlayerAction action, float amount);
	
	void tick();
	
	bool createWinlist(Table *t, std::vector< std::vector<HandStrength> > &winlist);
	
	int handleTable(Table *t);
	void stateNewRound(Table *t);
	void stateBlinds(Table *t);
	void stateBetting(Table *t);
	void stateAllFolded(Table *t);
	void stateShowdown(Table *t);
	void stateEndRound(Table *t);
	
	void dealHole(Table *t);
	void dealFlop(Table *t);
	void dealTurn(Table *t);
	void dealRiver(Table *t);
	
	bool isAllin(Table *t);
	
	void sendTableSnapshot(Table *t);
	
#ifdef DEBUG
	void setPlayerStake(int cid, float stake) { findPlayer(cid)->stake = stake; };
#endif
	
private:
	int game_id;
	
	bool started;
	unsigned int max_players;
	
	GameType type;
	
	std::vector<Player> players;
	std::vector<Table> tables;
	
	time_t timeout_start;
};


#endif /* _GAMECONTROLLER_H */
