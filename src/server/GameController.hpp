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
#include <map>
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
	bool getPlayerList(std::vector<int> &client_list) const;
	bool getPlayerList(int tid, std::vector<int> &client_list);
	
	bool addPlayer(int client_id);
	bool removePlayer(int client_id);
	
	void chat(int tid, const char* msg);
	void chat(int cid, int tid, const char* msg);
	
	bool setPlayerAction(int cid, Player::PlayerAction action, float amount);
	
	void tick();
	
#ifdef DEBUG
	void setPlayerStake(int cid, float stake) { findPlayer(cid)->stake = stake; };
#endif
	
protected:
	Player* findPlayer(int cid);
		
	void snap(int tid, int sid, const char* msg);
	void snap(int cid, int tid, int sid, const char* msg);
	
	bool createWinlist(Table *t, std::vector< std::vector<HandStrength> > &winlist);
	
	int handleTable(Table *t);
	void stateGameStart(Table *t);
	void stateNewRound(Table *t);
	void stateBlinds(Table *t);
	void stateBetting(Table *t);
	void stateAskShow(Table *t);
	void stateAllFolded(Table *t);
	void stateShowdown(Table *t);
	void stateEndRound(Table *t);
	
	void dealHole(Table *t);
	void dealFlop(Table *t);
	void dealTurn(Table *t);
	void dealRiver(Table *t);
	
	void sendTableSnapshot(Table *t);
	
private:
	int game_id;
	
	bool started;
	unsigned int max_players;
	
	GameType type;
	
	std::vector<Player> players;
	std::map<int,Table> tables;
	
	time_t game_start;
	time_t round_start;
	time_t betround_start;
	time_t timeout_start;
};


#endif /* _GAMECONTROLLER_H */
