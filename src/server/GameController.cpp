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


#include <cstdio>
#include <string>
#include <cstring>
#include <algorithm>

#include "Config.h"
#include "Logger.h"
#include "Debug.h"
#include "GameController.hpp"
#include "GameLogic.hpp"
#include "Card.hpp"

#include "game.hpp"


using namespace std;

// temporary buffer for chat/snap data
static char msg[1024];


GameController::GameController()
{
	reset();
	
	max_players = 10;
	restart = false;
	
	player_stakes = 1500;
	
	blind.blinds_time = 60 * 4;
	blind.blinds_factor = 20;
	blind.start = 10;
	
	name = "game";
	password = "";
	owner = -1;
}

GameController::GameController(const GameController& g)
{
	reset();
	
	setName(g.getName());
	setRestart(g.getRestart());
	setOwner(g.getOwner());
	//setGameType()
	setPlayerMax(g.getPlayerMax());
	setPlayerTimeout(g.getPlayerTimeout());
	setPlayerStakes(g.getPlayerStakes());
	setBlindsStart(g.getBlindsStart());
	setBlindsFactor(g.getBlindsFactor());
	setBlindsTime(g.getBlindsTime());
	setPassword(g.getPassword());
}

GameController::~GameController()
{
	// remove all players
	for (players_type::iterator e = players.begin(); e != players.end();)
	{
		delete e->second;
		players.erase(e++);
	}
}

void GameController::reset()
{
	game_id = -1;
	
	type = SNG;	// FIXME
	blind.blindrule = BlindByTime;	// FIXME:
	
	started = false;
	ended = false;
	finished = false;
	
	hand_no = 0;
	
	// remove all players
	for (players_type::iterator e = players.begin(); e != players.end();)
	{
		delete e->second;
		players.erase(e++);
	}
	
	// remove all spectators
	spectators.clear();
	
	// clear finish list
	finish_list.clear();
}

bool GameController::addPlayer(int cid, const std::string &uuid)
{
	// is the game already started or full?
	if (started || players.size() == max_players)
		return false;
	
	// is the client already a player?
	if (isPlayer(cid))
		return false;
	
	// remove from spectators list as we would receive the snapshots twice
	if (isSpectator(cid))
		removeSpectator(cid);
	
	Player *p = new Player;
	p->client_id = cid;
	p->stake = player_stakes;
	
	// save a copy of the UUID (player might disconnect)
	p->uuid = uuid;
	
	players[cid] = p;
	
	return true;
}

bool GameController::removePlayer(int cid)
{
	// don't allow removing if game has already been started
	if (started)
		return false;
	
	
	players_type::iterator it = players.find(cid);
	
	if (it == players.end())
		return false;
	
	bool bIsOwner = false;
	if (owner == cid)
		bIsOwner = true;
	
	delete it->second;
	
	players.erase(it);
	
	
	// find a new owner
	if (bIsOwner)
		selectNewOwner();
	
	return true;
}

bool GameController::isPlayer(int cid) const
{
	players_type::const_iterator it = players.find(cid);
	
	if (it == players.end())
		return false;
	
	return true;
}

bool GameController::addSpectator(int cid)
{
	// is the client already a spectator (or a player)?
	if (isSpectator(cid) || isPlayer(cid))
		return false;
	
	spectators.insert(cid);
	
	return true;
}

bool GameController::removeSpectator(int cid)
{
	spectators_type::iterator it = spectators.find(cid);
	
	if (it == spectators.end())
		return false;
	
	spectators.erase(it);
	
	return true;
}

bool GameController::isSpectator(int cid) const
{
	spectators_type::const_iterator it = spectators.find(cid);
	
	if (it == spectators.end())
		return false;
	
	return true;
}

Player* GameController::findPlayer(int cid)
{
	players_type::const_iterator it = players.find(cid);
	
	if (it == players.end())
		return NULL;
	
	return it->second;
}

bool GameController::setPlayerMax(unsigned int max)
{
	if (max < 2)
		return false;
	
	max_players = max;
	return true;
}

bool GameController::setPlayerStakes(chips_type stake)
{
	if (!stake)
		return false;
	
	player_stakes = stake;
	
	return true;
}

bool GameController::getPlayerList(vector<int> &client_list) const
{
	client_list.clear();
	
	for (players_type::const_iterator e = players.begin(); e != players.end(); e++)
		client_list.push_back(e->first);
	
	return true;
}

bool GameController::getListenerList(vector<int> &client_list) const
{
	client_list.clear();
	
	for (players_type::const_iterator e = players.begin(); e != players.end(); e++)
		client_list.push_back(e->first);
	
	for (spectators_type::const_iterator e = spectators.begin(); e != spectators.end(); e++)
		client_list.push_back(*e);
	
	return true;
}

void GameController::getFinishList(vector<Player*> &player_list) const
{
	for (finish_list_type::const_iterator e = finish_list.begin(); e != finish_list.end(); e++)
		player_list.push_back(*e);
}

void GameController::selectNewOwner()
{
	players_type::const_iterator e = players.begin();
	if (e == players.end())
		return;
	
	owner = e->second->client_id;
}

void GameController::chat(int tid, const char* msg)
{
	// players
	for (players_type::const_iterator e = players.begin(); e != players.end(); e++)
		client_chat(game_id, tid, e->first, msg);
	
	// spectators
	for (spectators_type::const_iterator e = spectators.begin(); e != spectators.end(); e++)
		client_chat(game_id, tid, *e, msg);
}

void GameController::chat(int cid, int tid, const char* msg)
{
	client_chat(game_id, tid, cid, msg);
}

void GameController::snap(int tid, int sid, const char* msg)
{
	// players
	for (players_type::const_iterator e = players.begin(); e != players.end(); e++)
		client_snapshot(game_id, tid, e->first, sid, msg);
	
	// spectators
	for (spectators_type::const_iterator e = spectators.begin(); e != spectators.end(); e++)
		client_snapshot(game_id, tid, *e, sid, msg);
}

void GameController::snap(int cid, int tid, int sid, const char* msg)
{
	client_snapshot(game_id, tid, cid, sid, msg);
}

bool GameController::setPlayerAction(int cid, Player::PlayerAction action, chips_type amount)
{
	Player *p = findPlayer(cid);
	
	if (!p)
		return false;
	
	if (action == Player::ResetAction)   // reset a previously set action
	{
		p->next_action.valid = false;
		return true;
	}
	else if (action == Player::Sitout)   // player wants to sit out
	{
		p->sitout = true;
		return true;
	}
	else if (action == Player::Back)     // player says "I'm back", end sitout
	{
		p->sitout = false;
		return true;
	}
	
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
		strength.setId(showdown_player);
		
		wl.push_back(strength);
		
		showdown_player = t->getNextActivePlayer(showdown_player);
	}
	
	return GameLogic::getWinList(wl, winlist);
}

void GameController::sendTableSnapshot(Table *t)
{
	// assemble community-cards string
	string scards;
	vector<Card> cards;
	
	t->communitycards.copyCards(&cards);
	
	for (unsigned int i=0; i < cards.size(); i++)
	{
		scards += cards[i].getName();
		
		if (i < cards.size() -1)
			scards += ':';
	}
	
	
	// assemble seats string
	string sseats;
	for (unsigned int i=0; i < 10; i++)
	{
		Table::Seat *s = &(t->seats[i]);
		
		if (!s->occupied)
			continue;
		
		Player *p = s->player;
		
		// assemble hole-cards string
		string shole;
		if (t->nomoreaction || s->showcards)
		{
			vector<Card> cards;
		
			p->holecards.copyCards(&cards);
			
			for (unsigned int i=0; i < cards.size(); i++)
				shole += cards[i].getName();
		}
		else
			shole = "-";
		
		int pstate = 0;
		if (s->in_round)
			pstate |= PlayerInRound;
		if (p->sitout)
			pstate |= PlayerSitout;
		
		char tmp[1024];
		snprintf(tmp, sizeof(tmp),
			"s%d:%d:%d:%d:%d:%d:%s",
			s->seat_no,
			p->client_id,
			pstate,
			p->stake,
			s->bet,
			p->last_action,
			shole.c_str());
		
		sseats += tmp;
		
		sseats += ' ';
	}
	
	
	// assemble pots string
	string spots;
	for (unsigned int i=0; i < t->pots.size(); i++)
	{
		Table::Pot *pot = &(t->pots[i]);
		
		char tmp[1024];
		
		snprintf(tmp, sizeof(tmp),
			"p%d:%d",
			i, pot->amount);
		
		spots += tmp;
		
		if (i < t->pots.size() -1)
			spots += ' ';
	}
	
	
	// assemble 'whose-turn' string
	string sturn;
	if (t->state == Table::GameStart ||
		t->state == Table::ElectDealer)
	{
		sturn = "-1";
	}
	else
	{
		char tmp[128];
		snprintf(tmp, sizeof(tmp), "%d:%d:%d:%d:%d",
			t->seats[t->dealer].seat_no,
			t->seats[t->sb].seat_no,
			t->seats[t->bb].seat_no,
			(t->cur_player == -1) ? -1 : (int)t->seats[t->cur_player].seat_no,
			t->seats[t->last_bet_player].seat_no);
		sturn = tmp;
	}
	
	
	chips_type minimum_bet;
	if (t->state == Table::Betting)
		minimum_bet = determineMinimumBet(t);
	else
		minimum_bet = 0;
	
	
	snprintf(msg, sizeof(msg),
		"%d:%d "           // <state>:<betting-round>
		"%s "              // <dealer>:<SB>:<BB>:<current>:<last-bet>
		"cc:%s "           // <community-cards>
		"%s "              // seats
		"%s "              // pots
		"%d",              // minimum bet
		t->state, (t->state == Table::Betting) ? t->betround : -1,
		sturn.c_str(),
		scards.c_str(),
		sseats.c_str(),
		spots.c_str(),
		minimum_bet);
	
	snap(t->table_id, SnapTable, msg);
}

void GameController::sendPlayerShowSnapshot(Table *t, Player *p)
{
	vector<Card> allcards;
	p->holecards.copyCards(&allcards);
	t->communitycards.copyCards(&allcards);
	
	string hsstr;
	for (vector<Card>::const_iterator e = allcards.begin(); e != allcards.end(); e++)
		hsstr += string(e->getName()) + string(" ");
	
	snprintf(msg, sizeof(msg), "%d %s",
		p->client_id,
		hsstr.c_str());
	
	snap(t->table_id, SnapPlayerShow, msg);
}

chips_type GameController::determineMinimumBet(Table *t) const
{
	if (t->bet_amount == 0)
		return blind.amount;
	else
		return t->bet_amount + (t->bet_amount - t->last_bet_amount);
}

void GameController::dealHole(Table *t)
{
	// player in small blind gets first cards
	for (unsigned int i = t->sb, c=0; c < t->countPlayers(); i = t->getNextPlayer(i))
	{
		if (!t->seats[i].occupied)
			continue;
		
		Player *p = t->seats[i].player;
		
		HoleCards h;
		Card c1, c2;
		t->deck.pop(c1);
		t->deck.pop(c2);
		p->holecards.setCards(c1, c2);
		
		char card1[3], card2[3];
		strcpy(card1, c1.getName());
		strcpy(card2, c2.getName());
		snprintf(msg, sizeof(msg), "%d %s %s",
			SnapCardsHole, card1, card2);
		snap(p->client_id, t->table_id, SnapCards, msg);
		
		
		// increase the found-player counter
		c++;
	}
}

void GameController::dealFlop(Table *t)
{
	Card f1, f2, f3;
	t->deck.pop(f1);
	t->deck.pop(f2);
	t->deck.pop(f3);
	t->communitycards.setFlop(f1, f2, f3);
	
	char card1[3], card2[3], card3[3];
	strcpy(card1, f1.getName());
	strcpy(card2, f2.getName());
	strcpy(card3, f3.getName());
	snprintf(msg, sizeof(msg), "%d %s %s %s",
		SnapCardsFlop, card1, card2, card3);
	snap(t->table_id, SnapCards, msg);
}

void GameController::dealTurn(Table *t)
{
	Card tc;
	t->deck.pop(tc);
	t->communitycards.setTurn(tc);
	
	char card[3];
	strcpy(card, tc.getName());
	snprintf(msg, sizeof(msg), "%d %s",
		SnapCardsTurn, card);
	snap(t->table_id, SnapCards, msg);
}

void GameController::dealRiver(Table *t)
{
	Card r;
	t->deck.pop(r);
	t->communitycards.setRiver(r);
	
	char card[3];
	strcpy(card, r.getName());
	snprintf(msg, sizeof(msg), "%d %s",
		SnapCardsRiver, card);
	snap(t->table_id, SnapCards, msg);
}

void GameController::stateNewRound(Table *t)
{
	// count up current hand number
	hand_no++;
	
	snprintf(msg, sizeof(msg), "%d %d", SnapGameStateNewHand, hand_no);
	snap(t->table_id, SnapGameState, msg);
	
#ifdef DEBUG
	log_msg("Table", "Hand #%d (gid=%d tid=%d)", hand_no, game_id, t->table_id);
#endif
	

#ifndef SERVER_TESTING
	// fill and shuffle card-deck
	t->deck.fill();
	t->deck.shuffle();
#else
	// set defined cards for testing
	if (debug_cards.size())
	{
		dbg_msg("deck", "using defined cards");
		t->deck.empty();
		t->deck.debugPushCards(&debug_cards);
	}
	else
	{
		dbg_msg("deck", "using random cards");
		t->deck.fill();
		t->deck.shuffle();
	}
#endif
	
	
	// reset round-related
	t->communitycards.clear();
	
	t->bet_amount = 0;
	t->last_bet_amount = 0;
	t->nomoreaction = false;
	
	// clear old pots and create initial main pot
	t->pots.clear();
	Table::Pot pot;
	pot.amount = 0;
	pot.final = false;
	t->pots.push_back(pot);
	
	
	// reset player-related
	for (unsigned int i = 0; i < 10; i++)
	{
		if (!t->seats[i].occupied)
			continue;
		
		t->seats[i].in_round = true;
		t->seats[i].showcards = false;
		t->seats[i].bet = 0;
		
		
		Player *p = t->seats[i].player;
		
		p->holecards.clear();
		p->resetLastAction();
		
		p->stake_before = p->stake;	// remember stake before this hand
	}
	
	
	// determine who is SB and BB
	bool headsup_rule = (t->countPlayers() == 2);
	
	if (headsup_rule)   // heads-up rule: only 2 players remain, so swap blinds
	{
		t->bb = t->getNextPlayer(t->dealer);
		t->sb = t->getNextPlayer(t->bb);
	}
	else
	{
		t->sb = t->getNextPlayer(t->dealer);
		t->bb = t->getNextPlayer(t->sb);
	}
	
	// player under the gun
	t->cur_player = t->getNextPlayer(t->bb);
	t->last_bet_player = t->cur_player;
	
	
	sendTableSnapshot(t);
	
	t->state = Table::Blinds;
}

void GameController::stateBlinds(Table *t)
{
	// new blinds level?
	switch ((int) blind.blindrule)
	{
	case BlindByTime:
		if (difftime(time(NULL), blind.last_blinds_time) > blind.blinds_time)
		{
			blind.last_blinds_time = time(NULL);
			blind.amount = (blind.blinds_factor * blind.amount) / 10;
			
			// send out blinds snapshot
			snprintf(msg, sizeof(msg), "%d %d %d", SnapGameStateBlinds, blind.amount / 2, blind.amount);
			snap(t->table_id, SnapGameState, msg);
		}
		break;
	}
	
	
	// FIXME: handle non-SNG correctly (ask each player for blinds ...)
	
	t->bet_amount = blind.amount;
	
	
	Player *pSmall = t->seats[t->sb].player;
	Player *pBig = t->seats[t->bb].player;
	
	
	// set the player's SB
	chips_type amount = blind.amount / 2;
	
	if (amount > pSmall->stake)
		amount = pSmall->stake;
	
	t->seats[t->sb].bet = amount;
	pSmall->stake -= amount;
	
	
	// set the player's BB
	amount = blind.amount;
	
	if (amount > pBig->stake)
		amount = pBig->stake;
	
	t->seats[t->bb].bet = amount;
	pBig->stake -= amount;
	
	
	// initialize the player's timeout
	t->timeout_start = time(NULL);
	
	
	// give out hole-cards
	dealHole(t);
	
#if 0
	// tell player 'under the gun' it's his turn
	Player *p = t->seats[t->cur_player].player;
	snap(p->client_id, t->table_id, SnapPlayerCurrent);
#endif
	
	// check if there is any more action possible
	if (t->isAllin())
	{
		if ((pBig->stake == 0 && pSmall->stake == 0) ||   // both players are allin
			(pBig->stake == 0 && t->seats[t->sb].bet >= t->seats[t->bb].bet) ||  // BB is allin, and SB has bet more or equal BB
			(pSmall->stake == 0))  // SB is allin
		{
			dbg_msg("no-more-action", "sb-allin:%s  bb-allin:%s",
				pSmall->stake ? "no" : "yes",
				pBig->stake ? "no" : "yes");
			t->nomoreaction = true;
		}
	}
	
	t->betround = Table::Preflop;
	t->scheduleState(Table::Betting, 3);
	
	sendTableSnapshot(t);
}

void GameController::stateBetting(Table *t)
{
	Player *p = t->seats[t->cur_player].player;
	
	bool allowed_action = false;  // is action allowed?
	bool auto_action = false;
	
	Player::PlayerAction action;
	chips_type amount = 0;
	
	chips_type minimum_bet = determineMinimumBet(t);
	
	if (t->nomoreaction ||		// early showdown, no more action at table possible, or
		p->stake == 0)		// player is allin and has no more options
	{
		action = Player::None;
		allowed_action = true;
	}
	else if (p->next_action.valid)  // has player set an action?
	{
		action = p->next_action.action;
		
		if (action == Player::Fold)
			allowed_action = true;
		else if (action == Player::Check)
		{
			// allowed to check?
			if (t->seats[t->cur_player].bet < t->bet_amount)
				chat(p->client_id, t->table_id, "You cannot check! Try call.");
			else
				allowed_action = true;
		}
		else if (action == Player::Call)
		{
			if (t->bet_amount == 0 || t->bet_amount == t->seats[t->cur_player].bet)
			{
				//chat(p->client_id, t->table_id, "You cannot call, nothing was bet! Try check.");
				
				// retry with this action
				p->next_action.action = Player::Check;
				return;
			}
			else if (t->bet_amount > t->seats[t->cur_player].bet + p->stake)
			{
				// simply convert this action to allin
				p->next_action.action = Player::Allin;
				return;
			}
			else
			{
				allowed_action = true;
				amount = t->bet_amount - t->seats[t->cur_player].bet;
			}
		}
		else if (action == Player::Bet)
		{
			if (t->bet_amount > 0)
				chat(p->client_id, t->table_id, "You cannot bet, there was already a bet! Try raise.");
			else if (p->next_action.amount < minimum_bet)
			{
				snprintf(msg, sizeof(msg), "You cannot bet this amount. Minimum bet is %d.",
					minimum_bet);
				chat(p->client_id, t->table_id, msg);
			}
			else
			{
				allowed_action = true;
				amount = p->next_action.amount - t->seats[t->cur_player].bet;
			}
		}
		else if (action == Player::Raise)
		{
			if (t->bet_amount == 0)
			{
				//chat(p->client_id, t->table_id, "Err: You cannot raise, nothing was bet! Try bet.");
				
				// retry with this action
				p->next_action.action = Player::Bet;
				return;
			}
			else if (p->next_action.amount < minimum_bet)
			{
				snprintf(msg, sizeof(msg), "You cannot raise this amount. Minimum bet is %d.",
					minimum_bet);
				chat(p->client_id, t->table_id, msg);
			}
			else
			{
				allowed_action = true;
				amount = p->next_action.amount - t->seats[t->cur_player].bet;
			}
		}
		else if (action == Player::Allin)
		{
			allowed_action = true;
			amount = p->stake;
		}
		
		// reset player action
		p->next_action.valid = false;
	}
	else
	{
		// handle player timeout
#ifndef SERVER_TESTING
		if (p->sitout || (unsigned int)difftime(time(NULL), t->timeout_start) > timeout)
		{
			// let player sit out (if not already sitting out)
			p->sitout = true;
			
			// auto-action: fold, or check if possible
			if (t->seats[t->cur_player].bet < t->bet_amount)
				action = Player::Fold;
			else
				action = Player::Check;
			
			allowed_action = true;
			auto_action = true;
		}
#endif /* SERVER_TESTING */
	}
	
	
	// return here if no or invalid action
	if (!allowed_action)
		return;
	
	
	// remember action for snapshot
	p->last_action = action;
	
	
	// perform action
	if (action == Player::None)
	{
		// do nothing
	}
	else if (action == Player::Fold)
	{
		t->seats[t->cur_player].in_round = false;
		
		snprintf(msg, sizeof(msg), "%d %d %d", SnapPlayerActionFolded, p->client_id, auto_action ? 1 : 0);
		snap(t->table_id, SnapPlayerAction, msg);
	}
	else if (action == Player::Check)
	{
		snprintf(msg, sizeof(msg), "%d %d %d", SnapPlayerActionChecked, p->client_id, auto_action ? 1 : 0);
		snap(t->table_id, SnapPlayerAction, msg);
	}
	else
	{
		// player can't bet/raise more than his stake
		if (amount > p->stake)
			amount = p->stake;
		
		// move chips from player's stake to seat-bet
		t->seats[t->cur_player].bet += amount;
		p->stake -= amount;
		
		if (action == Player::Bet || action == Player::Raise || action == Player::Allin)
		{
			// only re-open betting round if amount greater than table-bet
			// FIXME: bug: other players need to do an action even on none-minimum-bet
			if (t->seats[t->cur_player].bet > t->bet_amount /*&& t->seats[t->cur_player].bet >= minimum_bet*/)
			{
				t->last_bet_player = t->cur_player;
				t->last_bet_amount = t->bet_amount;     // needed for minimum-bet
				t->bet_amount = t->seats[t->cur_player].bet;
			}
			
			if (action == Player::Allin || p->stake == 0)
				snprintf(msg, sizeof(msg), "%d %d %d", SnapPlayerActionAllin, p->client_id, t->seats[t->cur_player].bet);
			else if (action == Player::Bet)
				snprintf(msg, sizeof(msg), "%d %d %d", SnapPlayerActionBet, p->client_id, t->bet_amount);
			else if (action == Player::Raise)
				snprintf(msg, sizeof(msg), "%d %d %d", SnapPlayerActionRaised, p->client_id, t->bet_amount);
		}
		else
			snprintf(msg, sizeof(msg), "%d %d %d", SnapPlayerActionCalled, p->client_id, amount);
		
		
		snap(t->table_id, SnapPlayerAction, msg);
	}
	
	// all players except one folded, so end this hand
	if (t->countActivePlayers() == 1)
	{
		// collect bets into pot
		t->collectBets();
		
		t->state = Table::AskShow;
		
		// set last remaining player as current player
		t->cur_player = t->getNextActivePlayer(t->cur_player);
		
		// initialize the player's timeout
		t->timeout_start = time(NULL);
		
		sendTableSnapshot(t);
		t->resetLastPlayerActions();
		return;
	}
	
	
	// is next the player who did the last bet/action? if yes, end this betting round
	if (t->getNextActivePlayer(t->cur_player) == t->last_bet_player)
	{
		// collect bets into pot
		t->collectBets();
		
		
		// all (or all except one) players are allin
		if (t->isAllin())
		{
			// no further action at table possible
			t->nomoreaction = true;
		}
		
		
		// which betting round is next?
		switch ((int)t->betround)
		{
		case Table::Preflop:
			t->betround = Table::Flop;
			dealFlop(t);
			break;
		
		case Table::Flop:
			t->betround = Table::Turn;
			dealTurn(t);
			break;
		
		case Table::Turn:
			t->betround = Table::River;
			dealRiver(t);
			break;
		
		case Table::River:
			// last_bet_player MUST show his hand
			t->seats[t->last_bet_player].showcards = true;
			
			// set the player behind last action as current player
			t->cur_player = t->getNextActivePlayer(t->last_bet_player);
			
			// initialize the player's timeout
			t->timeout_start = time(NULL);
			
			
			// end of hand, do showdown/ ask for show
			if (t->nomoreaction)
				t->state = Table::Showdown;
			else
				t->state = Table::AskShow;
			
			sendTableSnapshot(t);
			
			t->resetLastPlayerActions();
			return;
		}
		
		// send helper-snapshot // FIXME: do this smarter
		t->cur_player = -1;	// invalidate current player
		sendTableSnapshot(t);
		
		
		// reset the highest bet-amount
		t->bet_amount = 0;
		t->last_bet_amount = 0;
		
		// set current player to SB (or next active behind SB)
		t->cur_player = t->getNextActivePlayer(t->dealer);
		
		// re-initialize the player's timeout
		t->timeout_start = time(NULL);
		
		
		// first action for next betting round is at this player
		t->last_bet_player = t->cur_player;
		
		t->resetLastPlayerActions();
		
		t->scheduleState(Table::BettingEnd, 2);
	}
	else
	{
		// preflop: if player on whom the last action was (e.g. UTG) folds,
		// assign 'last action' to next active player
		if (action == Player::Fold && t->cur_player == t->last_bet_player)
			t->last_bet_player = t->getNextActivePlayer(t->last_bet_player);
		
		// find next player
		t->cur_player = t->getNextActivePlayer(t->cur_player);
		t->timeout_start = time(NULL);
		
		// reset current player's last action
		p = t->seats[t->cur_player].player;
		p->resetLastAction();
		
		t->scheduleState(Table::Betting, 1);
		sendTableSnapshot(t);
	}
	
#if 0
	// tell player it's his turn
	p = t->seats[t->cur_player].player;
	if (!t->nomoreaction && p->stake > 0)
		snap(p->client_id, t->table_id, SnapPlayerCurrent);
#endif
}

void GameController::stateBettingEnd(Table *t)
{
	t->state = Table::Betting;
	sendTableSnapshot(t);
}

void GameController::stateAskShow(Table *t)
{
	bool chose_action = false;
	
	Player *p = t->seats[t->cur_player].player;
	
	if (!p->stake && t->countActivePlayers() > 1) // player went allin and has no option to show/muck
	{
		t->seats[t->cur_player].showcards = true;
		chose_action = true;
		p->next_action.valid = false;
	}
	else if (p->next_action.valid)  // has player set an action?
	{
		if (p->next_action.action == Player::Muck)
		{
			// muck cards
			chose_action = true;
		}
		else if (p->next_action.action == Player::Show)
		{
			// show cards
			t->seats[t->cur_player].showcards = true;
			
			chose_action = true;
		}
		
		// reset scheduled action
		p->next_action.valid = false;
	}
	else
	{
#ifndef SERVER_TESTING
		// handle player timeout
		const int timeout = 4;   // FIXME: configurable
		if ((int)difftime(time(NULL), t->timeout_start) > timeout || p->sitout)
		{
			// default on showdown is "to show"
			// Note: client needs to determine if it's hand is
			//       already lost and needs to fold if wanted
			if (t->countActivePlayers() > 1)
				t->seats[t->cur_player].showcards = true;
			
			chose_action = true;
		}
#else /* SERVER_TESTING */
		t->seats[t->cur_player].showcards = true;
		chose_action = true;
#endif /* SERVER_TESTING */
	}
	
	// return here if no action chosen till now
	if (!chose_action)
		return;
	
	
	// remember action for snapshot
	if (t->seats[t->cur_player].showcards)
		p->last_action = Player::Show;
	else
		p->last_action = Player::Muck;
	
	
	// all-players-(except-one)-folded or showdown?
	if (t->countActivePlayers() == 1)
	{
		t->state = Table::AllFolded;
		
		//sendTableSnapshot(t);
	}
	else
	{
		// player is out if he don't want to show his cards
		if (t->seats[t->cur_player].showcards == false)
			t->seats[t->cur_player].in_round = false;
		
		
		if (t->getNextActivePlayer(t->cur_player) == t->last_bet_player)
		{
			t->state = Table::Showdown;
			return;
		}
		else
		{
			// find next player
			t->cur_player = t->getNextActivePlayer(t->cur_player);
			
			t->timeout_start = time(NULL);
			
			// send update snapshot
			sendTableSnapshot(t);
		}
	}
}

void GameController::stateAllFolded(Table *t)
{
	// get last remaining player
	Player *p = t->seats[t->cur_player].player;
	
	// send PlayerShow snapshot if cards were shown
	if (t->seats[t->cur_player].showcards)
		sendPlayerShowSnapshot(t, p);
	
	
	p->stake += t->pots[0].amount;
	t->seats[t->cur_player].bet = t->pots[0].amount;
	
	// send pot-win snapshot
	snprintf(msg, sizeof(msg), "%d %d %d", p->client_id, 0, t->pots[0].amount);
	snap(t->table_id, SnapWinPot, msg);
	
	
	sendTableSnapshot(t);
	t->scheduleState(Table::EndRound, 2);
}

void GameController::stateShowdown(Table *t)
{
	// the player who did the last action is first
	unsigned int showdown_player = t->last_bet_player;
	
	// determine and send out PlayerShow snapshots
	for (unsigned int i=0; i < t->countActivePlayers(); i++)
	{
		if (t->seats[showdown_player].showcards || t->nomoreaction)
		{
			Player *p = t->seats[showdown_player].player;
			
			sendPlayerShowSnapshot(t, p);
		}
		
		showdown_player = t->getNextActivePlayer(showdown_player);
	}
	
	
	// determine winners
	vector< vector<HandStrength> > winlist;
	createWinlist(t, winlist);
	
	// for each winner-list
	for (unsigned int i=0; i < winlist.size(); i++)
	{
		vector<HandStrength> &tw = winlist[i];
		const unsigned int winner_count = tw.size();
		
		// for each pot
		for (unsigned int poti=0; poti < t->pots.size(); poti++)
		{
			Table::Pot *pot = &(t->pots[poti]);
			const unsigned int involved_count = t->getInvolvedInPotCount(pot, tw);
			
			
			chips_type win_amount = 0;
			chips_type odd_chips = 0;
			
			if (involved_count)
			{
				// pot is divided by number of players involved in
				win_amount = pot->amount / involved_count;
				
				// odd chips
				odd_chips = pot->amount - (win_amount * involved_count);
			}
			
			
			chips_type cashout_amount = 0;
			
			// for each winning-player
			for (unsigned int pi=0; pi < winner_count; pi++)
			{
				const unsigned int seat_num = tw[pi].getId();
				Table::Seat *seat = &(t->seats[seat_num]);
				Player *p = seat->player;
				
				// skip pot if player not involved in it
				if (!t->isSeatInvolvedInPot(pot, seat_num))
					continue;
#if 0
				dbg_msg("winlist", "wl #%d involved-count=%d player #%d (seat:%d) pot #%d ($%d) ",
					i+1, involved_count, pi, seat_num, poti, pot->amount);
#endif
				
				if (win_amount > 0)
				{
					// transfer winning amount to player
					p->stake += win_amount;
					
					// put winnings to seat (needed for snapshot)
					seat->bet += win_amount;
					
					// count up overall cashed-out
					cashout_amount += win_amount;
					
					snprintf(msg, sizeof(msg), "%d %d %d", p->client_id, poti, win_amount);
					snap(t->table_id, SnapWinPot, msg);
				}
			}
			
			// distribute odd chips
			if (odd_chips)
			{
				// find the next player behind button which is involved in pot
				unsigned int oddchips_player = t->getNextActivePlayer(t->dealer);
				
				while (!t->isSeatInvolvedInPot(pot, oddchips_player))
					oddchips_player = t->getNextActivePlayer(oddchips_player);
				
				
				Table::Seat *seat = &(t->seats[oddchips_player]);
				Player *p = seat->player;
				
				p->stake += odd_chips;
				seat->bet += odd_chips;
				
				snprintf(msg, sizeof(msg), "%d %d %d", p->client_id, poti, odd_chips);
				snap(t->table_id, SnapOddChips, msg);
				
				cashout_amount += odd_chips;
			}
			
			// reduce pot about the overall cashed-out
			pot->amount -= cashout_amount;
		}
	}
	

#if 1
	// check for fatal error: not all pots were distributed
	for (unsigned int i=0; i < t->pots.size(); i++)
	{
		Table::Pot *pot = &(t->pots[i]);
		
		if (pot->amount > 0)
		{
			log_msg("winlist", "error: remaining chips in pot %d: %d",
				i, pot->amount);
		}
	}
#endif

	
	// reset all pots
	t->pots.clear();
	
	sendTableSnapshot(t);
	
	t->scheduleState(Table::EndRound, 2);
}

void GameController::stateEndRound(Table *t)
{
	multimap<chips_type,unsigned int> broken_players;
	
	// find broken players
	for (unsigned int i=0; i < 10; i++)
	{
		if (!t->seats[i].occupied)
			continue;
		
		Player *p = t->seats[i].player;
		
		// player has no stake left
		if (p->stake == 0)
			broken_players.insert(pair<chips_type,int>(p->stake_before, i));
		else
		{
			// there is a net win
			if (p->stake > p->stake_before)
			{
				snprintf(msg, sizeof(msg), "%d %d %d",
					p->client_id,
					-1,	/* reserved */
					p->stake - p->stake_before);
				snap(t->table_id, SnapWinAmount, msg);
			}
		}
	}
	
	// remove players in right order: sorted by stake_before
	// FIXME: how to handle players which had the same stake?
	for (multimap<chips_type,unsigned int>::iterator n = broken_players.begin(); n != broken_players.end(); n++)
	{
		//const chips_type stake_before = n->first;
		const unsigned int seat_num = n->second;
		
		Player *p = t->seats[seat_num].player;
		
		// save finish position
		finish_list.push_back(p);
		
		// send out player-broke snapshot
		snprintf(msg, sizeof(msg), "%d %d %d",
			SnapGameStateBroke,
			p->client_id,
			getPlayerCount() - (int)finish_list.size() + 1);
		
		snap(t->table_id, SnapGameState, msg);
		
		
		// mark seat as unused
		t->seats[seat_num].occupied = false;
	}
	
	
	sendTableSnapshot(t);
	
	
	// determine next dealer
	t->dealer = t->getNextPlayer(t->dealer);
	
	t->scheduleState(Table::NewRound, 2);
}

void GameController::stateDelay(Table *t)
{
#ifndef SERVER_TESTING
	if ((unsigned int) difftime(time(NULL), t->delay_start) >= t->delay)
		t->delay = 0;
#else
	t->delay = 0;
#endif
}

int GameController::handleTable(Table *t)
{
	if (t->delay)
	{
		stateDelay(t);
		return 0;
	}
	
	if (t->state == Table::NewRound)
		stateNewRound(t);
	else if (t->state == Table::Blinds)
		stateBlinds(t);
	else if (t->state == Table::Betting)
		stateBetting(t);
	else if (t->state == Table::BettingEnd)
		stateBettingEnd(t);
	else if (t->state == Table::AskShow)
		stateAskShow(t);
	else if (t->state == Table::AllFolded)
		stateAllFolded(t);
	else if (t->state == Table::Showdown)
		stateShowdown(t);
	else if (t->state == Table::EndRound)
		stateEndRound(t);
	
	
	// only 1 player left? close table
	if (t->countPlayers() == 1)
		return -1;
	
	return 0;
}

void GameController::start()
{
	// at least 2 players needed
	if (started || players.size() < 2)
		return;
	
	
	log_msg("game", "game %d has been started", game_id);
	
	started = true;
	
	// TODO: support more than 1 table
	const int tid = 0;
	Table *t = new Table();
	t->setTableId(tid);
	
	memset(t->seats, 0, sizeof(Table::Seat) * 10);
	
	
	// place players at table
	vector<Player*> rndseats;
	
	for (players_type::const_iterator e = players.begin(); e != players.end(); ++e)
		rndseats.push_back(e->second);
	
#ifndef SERVER_TESTING
	random_shuffle(rndseats.begin(), rndseats.end());
#endif
	
	for (unsigned int i=0; i < 10; i++)
	{
		Table::Seat *seat = &(t->seats[i]);
		
		seat->seat_no = i;
		seat->occupied = false;
	}
	
	
	bool chose_dealer = false;
	
	const int placement[10][10] = {
		{ 4 },					//  1 player
		{ 4, 9 },				//  2 players
		{ 4, 8, 0 },				//  3 players
		{ 3, 5, 8, 0 },				//  4 players
		{ 4, 6, 8, 0, 2 },			//  5 players
		{ 1, 2, 4, 6, 7, 9 },			//  6 players
		{ 4, 6, 2, 7, 1, 8, 0 },		//  7 players
		{ 1, 2, 3, 5, 6, 7, 8, 0 },		//  8 players
		{ 4, 6, 2, 7, 1, 8, 0, 5, 3 },		//  9 players
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }	// 10 players
	};
	
	const unsigned int place_row = players.size() - 1;
	unsigned int place_idx = 0;
	vector<Player*>::const_iterator it = rndseats.begin();
	
	do
	{
		const unsigned int place = placement[place_row][place_idx];
		Table::Seat *seat = &(t->seats[place]);
		const Player *p = *it;
		
		dbg_msg("placing", "place_row=%d place_idx=%d place=%d player=%d",
			place_row, place_idx, place, p->client_id);
		
		seat->occupied = true;
		seat->player = (Player*)p;
		
		// FIXME: implement choosing dealer correctly
		if (!chose_dealer)
		{
			t->dealer = place;
			chose_dealer = true;
		}
		
		it++;
	} while (++place_idx <= place_row);
	
	
	t->state = Table::GameStart;
	tables[tid] = t;
	
	blind.amount = blind.start;
	blind.last_blinds_time = time(NULL);
	
	snprintf(msg, sizeof(msg), "%d", SnapGameStateStart);
	snap(tid, SnapGameState, msg);
	
	sendTableSnapshot(t);
	
	t->scheduleState(Table::NewRound, 5);
}

int GameController::tick()
{
	if (!started)
	{
		if (getPlayerCount() == max_players)  // start game if player count reached
			start();
		else if (!getPlayerCount() && !getRestart())  // delete game if no players registered
			return -1;
		else	// nothing to do, exit early
			return 0;
	}
	else if (ended)
	{
		// delay before game gets deleted
		if ((unsigned int) difftime(time(NULL), ended_time) >= 4 * 60)
		{
			return -1;
		}
		else
			return 1;
	}
	
	// handle all tables
	for (tables_type::iterator e = tables.begin(); e != tables.end();)
	{
		Table *t = e->second;
		
		// table closed?
		if (handleTable(t) < 0)
		{
			// is this the last table?  /* FIXME: very very dirty */
			if (tables.size() == 1)
			{
				ended = true;
				ended_time = time(NULL);
				
				snprintf(msg, sizeof(msg), "%d", SnapGameStateEnd);
				snap(-1, SnapGameState, msg);
				
				// push back last remaining player to finish_list
				for (unsigned int i=0; i < 10; ++i)
					if (t->seats[i].occupied)
					{
						finish_list.push_back(t->seats[i].player);
						break;
					}
			}
			
			delete t;
			tables.erase(e++);
		}
		else
			++e;
	}
	
	return 0;
}
