#include <cstdio>
#include <string>
#include <algorithm>

#include "Config.h"
#include "Debug.h"
#include "GameController.hpp"
#include "GameLogic.hpp"

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

bool GameController::createWinlist(Table *t, vector< vector<HandStrength> > &winlist)
{
	vector<HandStrength> wl;
	
	unsigned int showdown_player = t->last_bet_player;
	for (unsigned int i=0; i < t->countActivePlayers(); i++)
	{
		Player *p = t->seats[showdown_player].player;
		
		HandStrength strength;
		GameLogic::getStrength(&(p->holecards), &(t->communitycards), &strength);
		strength.setId(p->client_id);
		
		wl.push_back(strength);
		
		showdown_player = t->getNextActivePlayer(showdown_player);
	}
	
	winlist.push_back(wl);
	
	int index=0;
	do
	{
		vector<HandStrength> &tw = winlist[index];
		vector<HandStrength> tmp;
		
		sort(tw.begin(), tw.end(), greater<HandStrength>());
		
		for (unsigned int i=tw.size()-1; i > 0; i--)
		{
			if (tw[i] < tw[0])
			{
				tmp.push_back(tw[i]);
				tw.pop_back();
			}
		}
		
		if (!tmp.size())
			break;
		
		winlist.push_back(tmp);
		index++;
		
	} while(true);
	
	return true;
}

// FIXME: SB gets first card; not very important because it doesn't really matter
void GameController::dealHole(Table *t)
{
	for (unsigned int i = t->cur_player, c=0; c < t->seats.size(); i = t->getNextPlayer(i), c++)
	{
		t->seats[i].in_round = true;
		Player *p = t->seats[i].player;
		
		HoleCards h;
		Card c1, c2;
		t->deck.pop(c1);
		t->deck.pop(c2);
		p->holecards.setCards(c1, c2);
		
		char card1[3], card2[3], msg[1024];
		strcpy(card1, c1.getName());
		strcpy(card2, c2.getName());
		snprintf(msg, sizeof(msg), "Your hole-cards: [%s %s]",
			card1, card2);
		
		client_chat(game_id, t->table_id, p->client_id, msg);
	}
}

void GameController::dealFlop(Table *t)
{
	Card f1, f2, f3;
	t->deck.pop(f1);
	t->deck.pop(f2);
	t->deck.pop(f3);
	t->communitycards.setFlop(f1, f2, f3);
	
	char card1[3], card2[3], card3[3], msg[1024];
	strcpy(card1, f1.getName());
	strcpy(card2, f2.getName());
	strcpy(card3, f3.getName());
	snprintf(msg, sizeof(msg), "The flop: [%s %s %s]",
		card1, card2, card3);
	
	chat(t->table_id, msg);
}

void GameController::dealTurn(Table *t)
{
	Card tc;
	t->deck.pop(tc);
	t->communitycards.setTurn(tc);
	
	char card[3], msg[1024];
	strcpy(card, tc.getName());
	snprintf(msg, sizeof(msg), "The turn: [%s]",
		card);
	
	chat(t->table_id, msg);
}

void GameController::dealRiver(Table *t)
{
	Card r;
	t->deck.pop(r);
	t->communitycards.setRiver(r);
	
	char card[3], msg[1024];
	strcpy(card, r.getName());
	snprintf(msg, sizeof(msg), "The river: [%s]",
		card);
	
	chat(t->table_id, msg);
}

void GameController::stateNewRound(Table *t)
{
	dbg_print("Table", "New Round");
	
	t->deck.fill();
	t->deck.shuffle();

#ifdef SERVER_TESTING
	chat(t->table_id, "---------------------------------------------");
	
	string ststr = "Stacks: ";
	for (unsigned int i=0; i < t->seats.size(); i++)
	{
		Player *p = t->seats[i].player;
		
		char tmp[128];
		snprintf(tmp, sizeof(tmp), "[%d]=%0.2f ", p->client_id, p->chipstack);
		ststr += tmp;
	}
	chat(t->table_id, ststr.c_str());
#endif
	
	chat(t->table_id, "New round started, deck shuffled");
	
	t->pot = 0.0f;
	t->state = Table::Blinds;
}

void GameController::stateBlinds(Table *t)
{
	// FIXME: handle non-SNG correctly (ask each player for blinds ...)
	
	t->bet_amount = (float)t->blind;
	
	bool headsup_rule = (t->seats.size() == 2);
	unsigned int small_blind, big_blind;
	
	// heads-up rule
	if (headsup_rule)
	{
		big_blind = t->getNextPlayer(t->dealer);
		small_blind = t->getNextPlayer(big_blind);
	}
	else
	{
		small_blind = t->getNextPlayer(t->dealer);
		big_blind = t->getNextPlayer(small_blind);
	}
	
	
	Player *pDealer = t->seats[t->dealer].player;
	Player *pSmall = t->seats[small_blind].player;
	Player *pBig = t->seats[big_blind].player;
	
	t->seats[small_blind].bet = t->blind / 2;
	pSmall->chipstack -= t->blind / 2;
	
	t->seats[big_blind].bet = t->blind;
	pBig->chipstack -= t->blind;
	
	char msg[1024];
	snprintf(msg, sizeof(msg), "[%d] is Dealer, [%d] is SB (%.2f), [%d] is BB (%.2f) %s",
		pDealer->client_id,
		pSmall->client_id, (float)t->blind / 2,
		pBig->client_id, (float)t->blind,
		(headsup_rule) ? "HEADS-UP" : "");
	chat(t->table_id, msg);
	
	// player under the gun
	t->cur_player = t->getNextPlayer(big_blind);
	
	// give out hole-cards
	dealHole(t);
	
	// tell current player
	Player *p = t->seats[t->cur_player].player;
	client_chat(game_id, t->table_id, p->client_id, "You're under the gun!");
	
	t->betround = Table::Preflop;
	t->last_bet_player = t->cur_player;
	
	t->state = Table::Betting;
}

void GameController::stateBetting(Table *t)
{
	char msg[1024];
	
	Player *p = t->seats[t->cur_player].player;
	bool allowed_action = false;  // is action allowed?
	
	Player::PlayerAction action;
	float amount;
	
	// has player set an action?
	if (p->next_action.valid)
	{
		action = p->next_action.action;
		
		dbg_print("game", "player %d choose an action", p->client_id);
		
		if (action == Player::Fold)
			allowed_action = true;
		else if (action == Player::Check)
		{
			// allowed to check?
			if (t->seats[t->cur_player].bet < t->bet_amount)
				client_chat(game_id, t->table_id, p->client_id, "Err: You cannot check! Try call.");
			else
				allowed_action = true;
		}
		else if (action == Player::Call)
		{
			if ((int)t->bet_amount == 0 || (int)t->bet_amount == t->seats[t->cur_player].bet)
				client_chat(game_id, t->table_id, p->client_id, "Err: You cannot call, nothing was bet! Try check.");
			else
			{
				allowed_action = true;
				amount = t->bet_amount - t->seats[t->cur_player].bet;
			}
		}
		else if (action == Player::Bet)
		{
			if ((unsigned int)t->bet_amount > 0)
				client_chat(game_id, t->table_id, p->client_id, "Err: You cannot bet, there was already a bet! Try raise.");
			else if (p->next_action.amount <= (unsigned int)t->bet_amount || p->next_action.amount < t->blind)
				client_chat(game_id, t->table_id, p->client_id, "Err: You cannot bet this amount.");
			else
			{
				allowed_action = true;
				amount = p->next_action.amount - t->seats[t->cur_player].bet;
			}
		}
		else if (action == Player::Raise)
		{
			if ((unsigned int)t->bet_amount == 0)
				client_chat(game_id, t->table_id, p->client_id, "Err: You cannot raise, nothing was bet! Try bet.");
			else if (p->next_action.amount <= (unsigned int)t->bet_amount)
				client_chat(game_id, t->table_id, p->client_id, "Err: You cannot raise this amount.");
			else
			{
				allowed_action = true;
				amount = p->next_action.amount - t->seats[t->cur_player].bet;
			}
		}
		else if (action == Player::Allin)
		{
			allowed_action = true;
			amount = p->chipstack;
		}
		
		// reset player action
		p->next_action.valid = false;
	}
	else
	{
		// FIXME: handle player timeout
	}
	
	
	// return here if no or invalid action
	if (!allowed_action)
		return;
	
	
	// perform action
	if (action == Player::Fold)
	{
		t->seats[t->cur_player].in_round = false;
		
		snprintf(msg, sizeof(msg), "Player %d folded.", p->client_id);
		chat(t->table_id, msg);
	}
	else if (action == Player::Check)
	{
		snprintf(msg, sizeof(msg), "Player %d checked.", p->client_id);
		chat(t->table_id, msg);
	}
	else
	{
		// player can't bet/raise more than this chipstack
		if (amount > p->chipstack)
			amount = p->chipstack;
		
		t->seats[t->cur_player].bet += amount;
		p->chipstack -= amount;
		
		if (action == Player::Bet || action == Player::Raise || (action == Player::Allin && amount > (unsigned int)t->bet_amount))
		{
			t->last_bet_player = t->cur_player;
			t->bet_amount = t->seats[t->cur_player].bet;
			
			snprintf(msg, sizeof(msg), "Player %d bet/raised/allin $%.2f.", p->client_id, amount);
		}
		else
			snprintf(msg, sizeof(msg), "Player %d called $%.2f.", p->client_id, amount);
		
		
		chat(t->table_id, msg);
	}
	
	// break here if only 1 player left
	if (t->countActivePlayers() == 1)
	{
		t->state = Table::AllFolded;
		return;
	}
	
	// is next the player who did the last bet/action?
	if (t->getNextActivePlayer(t->cur_player) == (int)t->last_bet_player)
	{
		dbg_print("table", "betting round ended");
		
		switch ((int)t->betround)
		{
		case Table::Preflop:
			// deal flop
			dealFlop(t);
			
			t->betround = Table::Flop;
			break;
		
		case Table::Flop:
			// deal turn
			dealTurn(t);
			
			t->betround = Table::Turn;
			break;
		
		case Table::Turn:
			// deal turn
			dealRiver(t);
			
			t->betround = Table::River;
			break;
		
		case Table::River:
			t->state = Table::Showdown;
			return;
		}
		
		// collect bets into pot
		for (unsigned int i=0; i < t->seats.size(); i++)
		{
			t->pot += t->seats[i].bet;
			t->seats[i].bet = 0.0f;
		}
		t->bet_amount = 0.0f;
		
#ifdef SERVER_TESTING
		// pot-size message
		snprintf(msg, sizeof(msg), "Pot size: %.2f", t->pot);
		chat(t->table_id, msg);
#endif
		
		// set current player to SB or next active behind SB
		bool headsup_rule = (t->seats.size() == 2);
		if (headsup_rule)
		{
			// FIXME: ugly
			t->cur_player = t->getNextActivePlayer(t->dealer);
			t->cur_player = t->getNextActivePlayer(t->cur_player);
		}
		else
			t->cur_player = t->getNextActivePlayer(t->dealer);
		
		// first action is at this player
		t->last_bet_player = t->cur_player;
		
		Player *p = t->seats[t->cur_player].player;
		client_chat(game_id, t->table_id, p->client_id, "It's your turn!");
	}
	else
	{
		// find next player
		t->cur_player = t->getNextActivePlayer(t->cur_player);
		Player *p = t->seats[t->cur_player].player;
		
		dbg_print("table", "next player");
		client_chat(game_id, t->table_id, p->client_id, "It's your turn!");
	}
}

void GameController::stateAllFolded(Table *t)
{
	// collect bets into pot
	for (unsigned int i=0; i < t->seats.size(); i++)
	{
		t->pot += t->seats[i].bet;
		t->seats[i].bet = 0.0f;
	}
	
	// get last remaining player
	Player *p = t->seats[t->getNextActivePlayer(t->cur_player)].player;
	
	// FIXME: ask player if he wants to show cards
	
	char msg[1024];
	snprintf(msg, sizeof(msg), "Player %d wins %.2f", p->client_id, t->pot);
	chat(t->table_id, msg);
	
	p->chipstack += t->pot;
	t->pot = 0.0f;
	
	t->state = Table::NewRound;
	t->dealer = t->getNextPlayer(t->dealer);
}

void GameController::stateShowdown(Table *t)
{
	char msg[1024];
	
	chat(t->table_id, "Showdown");
	
	// the player who did the last action is first to show
	unsigned int showdown_player = t->last_bet_player;
	
	// FIXME: ask for showdown if player has the option
	for (unsigned int i=0; i < t->countActivePlayers(); i++)
	{
		Player *p = t->seats[showdown_player].player;
		
		HandStrength strength;
		GameLogic::getStrength(&(p->holecards), &(t->communitycards), &strength);
		
		vector<Card> cards;
		string hsstr = "rank: ";
		
		cards.clear();
		strength.copyRankCards(&cards);
		for (vector<Card>::iterator e = cards.begin(); e != cards.end(); e++)
		{
			sprintf(msg, "%s ", e->getName());
			hsstr += msg;
		}
		
		hsstr += "kicker: ";
		cards.clear();
		strength.copyKickerCards(&cards);
		for (vector<Card>::iterator e = cards.begin(); e != cards.end(); e++)
		{
			sprintf(msg, "%s ", e->getName());
			hsstr += msg;
		}
		
		snprintf(msg, sizeof(msg), "Player [%d] has: %s (%s)",
			p->client_id,
			HandStrength::getRankingName(strength.getRanking()),
			hsstr.c_str());
		chat(t->table_id, msg);
		
		showdown_player = t->getNextActivePlayer(showdown_player);
	}
	
	vector< vector<HandStrength> > winlist;
	createWinlist(t, winlist);
	
	for (unsigned int i=0; i < winlist.size(); i++)
	{
		dbg_print("showdown", "--- Winlist %d---", i);
		
		vector<HandStrength> &tw = winlist[i];
		
		// FIXME: support case where further winlists are needed
		unsigned int winner_count = tw.size();
		float win_amount = t->pot / winner_count;
		
		for (unsigned int j=0; j < winner_count; j++)
		{
			Player *p = findPlayer(tw[j].getId());
			p->chipstack += win_amount;
			
			snprintf(msg, sizeof(msg), "Winner is [%d] and wins %.2f",
				p->client_id, win_amount);
			
			chat(t->table_id, msg);
		}
		
		// FIXME: support case where further winlists are needed
		t->pot = 0.0f;
		break;
	}
	
	t->state = Table::NewRound;
	t->dealer = t->getNextPlayer(t->dealer);
}

int GameController::handleTable(Table *t)
{
	if (t->state == Table::ElectDealer)
	{
		// FIXME: implement me
	}
	else if (t->state == Table::NewRound)
		stateNewRound(t);
	else if (t->state == Table::Blinds)
		stateBlinds(t);
	else if (t->state == Table::Betting)
		stateBetting(t);
	else if (t->state == Table::AllFolded)
		stateAllFolded(t);
	else if (t->state == Table::Showdown)
		stateShowdown(t);
	
	return 0;
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
	for (unsigned int i=0; i < tables.size(); i++)
	{
		handleTable(&(tables[i]));
	}
}
