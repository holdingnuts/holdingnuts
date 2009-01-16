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


#include "Debug.h"
#include "Table.hpp"

using namespace std;


Table::Table()
{
	table_id = -1;
}

int Table::getNextPlayer(unsigned int pos)
{
	unsigned int start = pos;
	unsigned int cur = pos;
	bool found = false;
	
	do
	{
		cur += 1;
		if (cur >= 10)
			cur = 0;
		
		// no active player left
		if (start == cur)
			return -1;
		
		if (seats[cur].occupied)
			found = true;
		
	} while (!found);
	
	return cur;
}

int Table::getNextActivePlayer(unsigned int pos)
{
	unsigned int start = pos;
	unsigned int cur = pos;
	bool found = false;
	
	do
	{
		cur = getNextPlayer(cur);
		
		// no active player left
		if (start == cur)
			return -1;
		
		if (seats[cur].in_round)
			found = true;
	} while (!found);
	
	return cur;
}

unsigned int Table::countPlayers()
{
	unsigned int count = 0;
	
	for (unsigned int i=0; i < 10; i++)
	{
		if (seats[i].occupied)
			count++;
	}
	
	return count;
}

unsigned int Table::countActivePlayers()
{
	unsigned int count = 0;
	
	for (unsigned int i=0; i < 10; i++)
	{
		if (seats[i].occupied && seats[i].in_round)
			count++;
	}
	
	return count;
}

// all (or except one) players are allin
bool Table::isAllin()
{
	unsigned int count = 0;
	unsigned int active_players = 0;
	
	for (unsigned int i=0; i < 10; i++)
	{
		if (seats[i].occupied && seats[i].in_round)
		{
			active_players++;
			
			Player *p = seats[i].player;
			
			if ((int)p->getStake() == 0)
				count++;
		}
	}
	
	return (count >= active_players - 1);
}

bool Table::isPlayerInvolvedInPot(Pot *pot, Player *p)
{
	for (unsigned int i=0; i < pot->players.size(); i++)
	{
		if (pot->players[i] == p)
			return true;
	}
	
	return false;
}

unsigned int Table::getInvolvedInPotCount(Pot *pot, std::vector<HandStrength> &wl)
{
	unsigned int involved_count = 0;
	
	for (unsigned int i=0; i < pot->players.size(); i++)
	{
		int cid = pot->players[i]->getClientId();
		
		for (unsigned int j=0; j < wl.size(); j++)
		{
			if (wl[j].getId() == cid)
				involved_count++;
		}
	}
	
	return involved_count;
}

void Table::collectBets()
{
	do
	{
		// find smallest bet
		float smallest_bet = 0.0f;
		bool need_sidepot = false;
		
		for (unsigned int i=0; i < 10; i++)
		{
			// skip folded and already handled players
			if (!seats[i].occupied || !seats[i].in_round || (int)seats[i].bet == 0)
				continue;
			
			if ((int)smallest_bet == 0)   // set an initial value
				smallest_bet = seats[i].bet;
			else if (seats[i].bet < smallest_bet)  // new smallest bet
			{
				smallest_bet = seats[i].bet;
				need_sidepot = true;
			}
			else if (seats[i].bet > smallest_bet)  // bets are not equal,
				need_sidepot = true;           // so there must be a smallest bet
		}
		
		
#if 0
		dbg_print("collectBets", "smallest_bet: %s = %.2f", need_sidepot ? "true" : "false", smallest_bet);
#endif
		// there are no bets, do nothing
		if ((int)smallest_bet == 0)
			return;
		
		
		// last pot is current pot
		Pot *cur_pot = &(pots[pots.size() - 1]);
		
		// if current pot is final, create a new one
		if (cur_pot->final)
		{
			Pot pot;
			pot.amount = 0.0f;
			pot.final = false;
			pots.push_back(pot);
			
			cur_pot = &(pots[pots.size() - 1]);
		}
		
		
		// collect the bet of each player
		for (unsigned int i=0; i < 10; i++)
		{
			// skip invalid seats
			if (!seats[i].occupied)
				continue;
			
			// skip already handled players
			if ((int)seats[i].bet == 0)
				continue;
			
			// collect bet of folded players and skip them
			if (!seats[i].in_round)
			{
				cur_pot->amount += seats[i].bet;
				seats[i].bet = 0.0f;
				continue;
			}
			
			// collect the bet into pot
			if (!need_sidepot)
			{
				cur_pot->amount += seats[i].bet;
				seats[i].bet = 0.0f;
			}
			else
			{
				cur_pot->amount += smallest_bet;
				seats[i].bet -= smallest_bet;
			}
			
			// mark pot as final if at least one player is allin
			Player *p = seats[i].player;
			if ((int)p->getStake() == 0)
				cur_pot->final = true;
			
			// set player 'involved in pot'
			if (!isPlayerInvolvedInPot(cur_pot, p))
				cur_pot->players.push_back(p);
		}
		
		
		if (!need_sidepot)  // all player bets are the same, end here
			break;
		
	} while (true);

#if 0
	for (unsigned int i=0; i < pots.size(); i++)
	{
		dbg_print("pot", "#%d: amount=%0.2f players=%d",
			i+1, pots[i].amount, (int)pots[i].players.size());
		
		for (unsigned int j=0; j < pots[i].players.size(); j++)
		{
			Player *p = pots[i].players[j];
			dbg_print("pot", "    player %d", p->getClientId());
		}
	}
	
	for (unsigned int i=0; i < seats.size(); i++)
	{
		dbg_print("seat-bets", "seat-%d: %.2f", i, seats[i].bet);
	}
#endif
}

float Table::determineMinimumBet() const
{
	if ((int) bet_amount == 0)
		return blind;
	else
		return bet_amount + (bet_amount - last_bet_amount);
}
