#include <cstdio>

#include "Config.h"
#include "Debug.h"
#include "GameController.hpp"

#include "game.hpp"


using namespace std;

extern clientcon* get_client_by_sock(socktype sock);
extern bool client_chat(int from_gid, int from_tid, int to, const char *msg);


GameController::GameController()
{
	game_id = -1;
	
	started = false;
	max_players = MAX_PLAYERS;
	
	type = SNG;
}

bool GameController::addPlayer(int client_id)
{
	if (started || players.size() == max_players)
		return false;
	
	Player p;
	p.client_id = client_id;
	p.chipstack = 1500.0f;
	p.next_action.valid = false;
	
	players.push_back(p);
	
	return true;
}

bool GameController::removePlayer(int client_id)
{
	if (started)
		return false;
		
	for (vector<Player>::iterator e = players.begin(); e != players.end(); e++)
	{
		if (e->client_id == client_id)
		{
			players.erase(e);
			return true;
		}
	}
	
	return false;
}

Player* GameController::findPlayer(int cid)
{
	for (unsigned int i=0; i < players.size(); i++)
	{
		if (players[i].client_id == cid)
			return &(players[i]);
	}
	
	return NULL;
}

bool GameController::setPlayerMax(unsigned int max)
{
	if (max < 2 || max > MAX_PLAYERS)
		return false;
	
	max_players = max;
	return true;
}

void GameController::chat(int tid, const char* msg)
{
	for (vector<Player>::iterator e = players.begin(); e != players.end(); e++)
		client_chat(game_id, tid, e->client_id, msg);
}

bool GameController::setPlayerAction(int cid, Player::PlayerAction action, float amount)
{
	Player *p = findPlayer(cid);
	
	if (!p)
		return false;
	
	p->next_action.valid = true;
	p->next_action.action = action;
	p->next_action.amount = amount;
	
	return true;
}

void GameController::tick()
{
	if (!started)
	{
		// start a game
		if (getPlayerCount() == max_players)
		{
			dbg_print("game", "game %d has been started", game_id);
			
			started = true;
			
			// FIXME: support more than 1 table
			const int tid = 0;
			Table table;
			table.setTableId(tid);
			for (unsigned int i=0; i < players.size(); i++)
			{
				Table::Seat seat;
				memset(&seat, 1, sizeof(Table::Seat));
				seat.player = &(players[i]);
				table.seats.push_back(seat);
			}
			table.dealer = 0;
			table.blind = 10;
			table.state = Table::NewRound;
			tables.push_back(table);
			
			char msg[1024];
			snprintf(msg, sizeof(msg), "Game %d has been started. You're at table %d.", game_id, tid);
			chat(-1, msg);
		}
		
		return;
	}
	
	// handle all tables
	for (vector<Table>::iterator e = tables.begin(); e != tables.end(); e++)
	{
		if (e->state == Table::NewRound)
		{
			dbg_print("Table", "New Round");
			
			e->deck.fill();
			e->deck.shuffle();
			
			chat(e->getTableId(), "New round started, deck shuffled");
			
			e->state = Table::Blinds;
		}
		else if (e->state == Table::Blinds)
		{
			// FIXME: handle non-SNG correctly (ask each player for blinds ...)
			dbg_print("Table", "Blinds");
			
			e->bet_amount = (float)e->blind;
			
			// FIXME: heads-up rule
			int small_blind = e->getNextPlayer(e->dealer);
			int big_blind = e->getNextPlayer(small_blind);
			
			Player *pSmall = e->seats[small_blind].player;
			Player *pBig = e->seats[big_blind].player;
			
			e->seats[small_blind].bet = e->blind / 2;
			pSmall->chipstack -= e->blind / 2;
			
			e->seats[big_blind].bet = e->blind;
			pBig->chipstack -= e->blind;
			
			char msg[1024];
			snprintf(msg, sizeof(msg), "Player %d is Small blind ($%d), Player %d is Big blind ($%d)",
				pSmall->client_id, e->blind / 2, pBig->client_id, e->blind);
			chat(e->getTableId(), msg);
			
			// player under the gun
			e->cur_player = e->getNextPlayer(big_blind);
			
			// give out hole-cards
			for (unsigned int i = e->cur_player, c=0; c < e->seats.size(); i = e->getNextPlayer(i), c++)
			{
				Player *p = e->seats[i].player;
				
				p->in_round = true;
				
				//dbg_print("table", "Hole cards for %d", p->client_id);
				
				HoleCards h;
				Card c1, c2;
				e->deck.pop(c1);
				e->deck.pop(c2);
				p->holecards.setCards(c1, c2);
				
				char card1[3], card2[3], msg[1024];
				strcpy(card1, c1.getName());
				strcpy(card2, c2.getName());
				snprintf(msg, sizeof(msg), "Your hole-cards: [%s %s]",
					card1, card2);
				
				client_chat(game_id, e->table_id, p->client_id, msg);
			}
			
			// tell current player
			Player *p = e->seats[e->cur_player].player;
			client_chat(game_id, e->table_id, p->client_id, "You're under the gun!");
			
			e->betround = Table::Preflop;
			e->last_bet_player = e->cur_player;
			
			e->state = Table::Betting;
		}
		else if (e->state == Table::Betting)
		{
			Player *p = e->seats[e->cur_player].player;
			bool valid_action = false;  // is action allowed?
			
			Player::PlayerAction action;
			float amount;
			
			// has player set an action?
			if (p->next_action.valid)
			{
				action = p->next_action.action;
				
				dbg_print("game", "player %d choose an action", p->client_id);
				
				if (action == Player::Fold)
					valid_action = true;
				else if (action == Player::Check)
				{
					// allowed to check?
					if (e->seats[e->cur_player].bet < e->bet_amount)
						client_chat(game_id, e->table_id, p->client_id, "Err: You cannot check! Try call.");
					else
						valid_action = true;
				}
				else if (action == Player::Call)
				{
					if ((int)e->bet_amount == 0 || (int)e->bet_amount == e->seats[e->cur_player].bet)
						client_chat(game_id, e->table_id, p->client_id, "Err: You cannot call, nothing was bet! Try check.");
					else
					{
						valid_action = true;
						amount = e->bet_amount;
					}
				}
				else if (action == Player::Bet)
				{
					if ((unsigned int)e->bet_amount > e->blind)
						client_chat(game_id, e->table_id, p->client_id, "Err: You cannot bet, there was already a bet! Try raise.");
					else
					{
						valid_action = true;
						amount = p->next_action.amount;
					}
				}
				else if (action == Player::Raise)
				{
					if ((unsigned int)e->bet_amount == 0 || (unsigned int)e->bet_amount == e->blind)
						client_chat(game_id, e->table_id, p->client_id, "Err: You cannot raise, nothing was bet! Try bet.");
					else
					{
						valid_action = true;
						amount = p->next_action.amount;
					}
				}
				else if (action == Player::Allin)
				{
					valid_action = true;
					amount = p->chipstack;
				}
				else
					valid_action = true;  // FIXME: debugging; fix this case!
				
				// reset
				p->next_action.valid = false;
			}
			else
			{
				// FIXME: handle player timeout
			}
			
			if (valid_action)
			{
				char msg[1024];
				
				// perform action
				if (action == Player::Fold)
				{
					p->in_round = false;
					
					snprintf(msg, sizeof(msg), "Player %d folded.", p->client_id);
					chat(e->getTableId(), msg);
				}
				else if (action == Player::Check)
				{
					snprintf(msg, sizeof(msg), "Player %d checked.", p->client_id);
					chat(e->getTableId(), msg);
				}
				else
				{
					e->seats[e->cur_player].bet += amount;
					p->chipstack -= amount;
					
					if (action == Player::Bet || action == Player::Raise || (action == Player::Allin && amount > (unsigned int)e->bet_amount))
					{
						e->last_bet_player = e->cur_player;
						e->bet_amount = amount;
						
						snprintf(msg, sizeof(msg), "Player %d bet/raised/allin $%.2f.", p->client_id, amount);
					}
					else
						snprintf(msg, sizeof(msg), "Player %d called $%.2f.", p->client_id, amount);
					
					
					chat(e->getTableId(), msg);
				}
				
				// is next the player who did the last bet/action?
				if (e->getNextPlayer(e->cur_player) == (int)e->last_bet_player)
				{
					dbg_print("table", "betting round ended");
					//if (last_round) e->dealer = e->getNextPlayer(e->dealer);
				}
				else
				{
					Player *p;
					bool found = false;
					do
					{
						e->cur_player = e->getNextPlayer(e->cur_player);
						
						p = e->seats[e->cur_player].player;
						if (p->in_round)
							found = true;
					} while (!found);
					
					dbg_print("table", "next player");
					client_chat(game_id, e->table_id, p->client_id, "It's your turn!");
				}
			}
			
		}
	}
}
