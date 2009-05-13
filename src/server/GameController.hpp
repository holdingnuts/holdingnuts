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


#ifndef _GAMECONTROLLER_H
#define _GAMECONTROLLER_H

#include <vector>
#include <map>
#include <string>
#include <ctime>

#include "Card.hpp"
#include "Deck.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "Table.hpp"
#include "Player.hpp"
#include "GameLogic.hpp"


class GameController
{
friend class TestCaseGameController;

public:
	typedef std::map<int,Table*>	tables_type;
	typedef std::map<int,Player*>	players_type;
	
	typedef enum {
		RingGame,   // Cash game
		FreezeOut,  // Tournament
		SNG         // Sit'n'Go
	} GameType;
	
	
	typedef enum {
		BlindByTime,
		//BlindByRound,
		BlindByKnockout
	} BlindRule;
	
	typedef enum {
		NoLimit,
		PotLimit,
		SplitLimit,
		FixedLimit
	} LimitRule;
	
	GameController();
	
	bool setGameId(int gid) { game_id = gid; return true; };
	int getGameId() const { return game_id; };
	
	GameType getGameType() const { return type; };
	
	void setPlayerTimeout(unsigned int respite) { timeout = respite; };
	unsigned int getPlayerTimeout() const { return timeout; };
	
	void setBlindsStart(chips_type blinds_start) { blind.start = blinds_start; };
	chips_type getBlindsStart() const { return blind.start; };
	void setBlindsFactor(float blinds_factor) { blind.blinds_factor = blinds_factor; };
	float getBlindsFactor() const { return blind.blinds_factor; };
	void setBlindsTime(unsigned int blinds_time) { blind.blinds_time = blinds_time; };
	unsigned int getBlindsTime() const { return blind.blinds_time; };
	
	bool setPlayerStakes(chips_type stake);
	chips_type getPlayerStakes() const { return player_stakes; };
	
	std::string getName() const { return name; };
	bool setName(const std::string &str) { name = str; return true; }; // FIXME: validate
	
	bool checkPassword(const std::string &passwd) const { return (!password.length() || password == passwd); };
	bool hasPassword() const { return password.length(); };
	bool setPassword(const std::string &str) { password = str; return true; };
	
	bool setPlayerMax(unsigned int max);
	unsigned int getPlayerMax() const { return max_players; };
	unsigned int getPlayerCount() const { return players.size(); };
	bool getPlayerList(std::vector<int> &client_list) const;
	
	void setRestart(bool bRestart) { restart = bRestart; };
	bool getRestart() const { return restart; };
	
	bool isStarted() const { return started; };
	bool isEnded() const { return ended; };
	
	bool addPlayer(int cid);
	bool removePlayer(int cid);
	bool isPlayer(int cid) const;
	
	void setOwner(int cid) { owner = cid; };
	int getOwner() const { return owner; };
	
	void chat(int tid, const char* msg);
	void chat(int cid, int tid, const char* msg);
	
	bool setPlayerAction(int cid, Player::PlayerAction action, chips_type amount);
	
	void start();
	
	int tick();
	
	
protected:
	Player* findPlayer(int cid);
	void selectNewOwner();
	
	void snap(int tid, int sid, const char* msg="");
	void snap(int cid, int tid, int sid, const char* msg="");
	
	bool createWinlist(Table *t, std::vector< std::vector<HandStrength> > &winlist);
	chips_type determineMinimumBet(Table *t) const;
	
	int handleTable(Table *t);
	void stateNewRound(Table *t);
	void stateBlinds(Table *t);
	void stateBetting(Table *t);
	void stateBettingEnd(Table *t);   // pseudo-state
	void stateAskShow(Table *t);
	void stateAllFolded(Table *t);
	void stateShowdown(Table *t);
	void stateEndRound(Table *t);
	
	// pseudo-state for delays
	void stateDelay(Table *t);
	
	void dealHole(Table *t);
	void dealFlop(Table *t);
	void dealTurn(Table *t);
	void dealRiver(Table *t);
	
	void sendTableSnapshot(Table *t);
	void sendPlayerShowSnapshot(Table *t, Player *p);
	
private:
	int game_id;
	
	bool started;
	unsigned int max_players;
	
	GameType type;
	LimitRule limit;
	chips_type player_stakes;
	unsigned int timeout;
	
	players_type players;
	tables_type tables;
	
	struct {
		chips_type start;
		chips_type amount;
		BlindRule blindrule;
		unsigned int blinds_time;  // seconds
		time_t last_blinds_time;
		float blinds_factor;
	} blind;
	
	unsigned int hand_no;
	
	int owner;   // owner of a game
	bool restart;   // should be restarted when ended?
	
	bool ended;
	time_t ended_time;
	
	std::string name;
	std::string password;
	
#ifdef DEBUG
	std::vector<Card> debug_cards;
#endif
};

#endif /* _GAMECONTROLLER_H */
