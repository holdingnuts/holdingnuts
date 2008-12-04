#ifndef _GAMECONTROLLER_H
#define _GAMECONTROLLER_H

#include <vector>

#include "Card.hpp"
#include "Deck.hpp"
#include "HoleCards.hpp"
#include "CommunityCards.hpp"
#include "Player.hpp"

// only variant "Texas Hold'em" supported yet

class GameController
{
public:
	typedef enum {
		ElectDealer,
		Blinds,
		Betting,
		Showdown,
	} State;
	
	typedef enum {
		Preflop,
		Flop,
		Turn,
		River
	} BettingRound;
	
	typedef enum {
		RingGame,   // Cash game
		FreezeOut,  // Tournament
		SNG         // Sit'n'Go
	} GameType;
	
	GameController();
	
	bool setGameId(int gid) { game_id = gid; return true; };
	int getGameId() { return game_id; };
	
	bool setPlayerMax(unsigned int max);
	unsigned int getPlayerMax() { return max_players; };
	unsigned int getPlayerCount() { return players.size(); };
	
	bool addPlayer(int client_id);
	bool removePlayer(int client_id);
	
	void tick();

private:
	int game_id;
	
	bool started;
	unsigned int max_players;
	
	State state;
	GameType type;
	Player *dealer;
	
	std::vector<Player> players;
};


#endif /* _GAMECONTROLLER_H */
