#ifndef _TABLE_H
#define _TABLE_H

#include "Deck.hpp"
#include "CommunityCards.hpp"
#include "Player.hpp"

class Table
{
friend class GameController;

public:
	typedef enum {
		ElectDealer,
		NewRound,
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
	
	typedef struct {
		Player *player;
		float bet;
		bool in_round;
	} Seat;
	
	Table();
	
	bool setTableId(int tid) { table_id = tid; return true; };
	int getTableId() { return table_id; };
	
	int getNextPlayer(unsigned int pos);
	int getNextActivePlayer(unsigned int pos);
	unsigned int countActivePlayers();
	
	void tick();
	
private:
	int table_id;
	
	Deck deck;
	CommunityCards communitycards;
	
	State state;
	BettingRound betround;
	
	unsigned int blind;
	
	std::vector<Seat> seats;
	unsigned int dealer;
	unsigned int cur_player;
	unsigned int last_bet_player;
	
	float bet_amount;
	float pot;
};


#endif /* _TABLE_H */
