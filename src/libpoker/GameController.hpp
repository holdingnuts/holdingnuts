#ifndef _GAMECONTROLLER_H
#define _GAMECONTROLLER_H

#include <vector>

#include "Card.hpp"
#include "Deck.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "Table.hpp"
#include "Player.hpp"


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
	
	bool setPlayerAction(int cid, Player::PlayerAction action, float amount);
	
	void tick();
	
	int handleTable(Table *t);
	void stateNewRound(Table *t);
	void stateBlinds(Table *t);
	void stateBetting(Table *t);
	void stateAllFolded(Table *t);
	void stateShowdown(Table *t);
	
	void dealHole(Table *t);
	void dealFlop(Table *t);
	void dealTurn(Table *t);
	void dealRiver(Table *t);

private:
	int game_id;
	
	bool started;
	unsigned int max_players;
	
	GameType type;
	
	std::vector<Player> players;
	std::vector<Table> tables;
};


#endif /* _GAMECONTROLLER_H */
