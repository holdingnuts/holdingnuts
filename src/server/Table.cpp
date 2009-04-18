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


#include "Logger.h"
#include "Debug.h"
#include "Table.hpp"

#include <ctime>

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
			
			if (p->getStake() == 0)
				count++;
		}
	}
	
	return (count >= active_players - 1);
}

bool Table::isSeatInvolvedInPot(Pot *pot, unsigned int s)
{
	for (unsigned int i=0; i < pot->vseats.size(); i++)
	{
		if (pot->vseats[i] == s)
			return true;
	}
	
	return false;
}

unsigned int Table::getInvolvedInPotCount(Pot *pot, std::vector<HandStrength> &wl)
{
	unsigned int involved_count = 0;
	
	for (unsigned int i=0; i < pot->vseats.size(); i++)
	{
		const unsigned int s = pot->vseats[i];
		
		for (unsigned int j=0; j < wl.size(); j++)
		{
			if ((unsigned int) wl[j].getId() == s)
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
		chips_type smallest_bet = 0;
		bool need_sidepot = false;
		
		for (unsigned int i=0; i < 10; i++)
		{
			// skip folded and already handled players
			if (!seats[i].occupied || !seats[i].in_round || seats[i].bet == 0)
				continue;
			
			if (smallest_bet == 0)   // set an initial value
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
		log_msg("collectBets", "smallest_bet: sidepot=%s with %d", 
			need_sidepot ? "true" : "false", smallest_bet);
#endif
		// there are no bets, do nothing
		if (smallest_bet == 0)
			return;
		
		
		// last pot is current pot
		Pot *cur_pot = &(pots[pots.size() - 1]);
		
		// if current pot is final, create a new one
		if (cur_pot->final)
		{
			Pot pot;
			pot.amount = 0;
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
			if (seats[i].bet == 0)
				continue;
			
			// collect bet of folded players and skip them
			if (!seats[i].in_round)
			{
				cur_pot->amount += seats[i].bet;
				seats[i].bet = 0;
				continue;
			}
			
			// collect the bet into pot
			if (!need_sidepot)
			{
				cur_pot->amount += seats[i].bet;
				seats[i].bet = 0;
			}
			else
			{
				cur_pot->amount += smallest_bet;
				seats[i].bet -= smallest_bet;
			}
			
			// mark pot as final if at least one player is allin
			Player *p = seats[i].player;
			if (p->getStake() == 0)
				cur_pot->final = true;
			
			// set player 'involved in pot'
			if (!isSeatInvolvedInPot(cur_pot, i))
				cur_pot->vseats.push_back(i);
		}
		
		
		if (!need_sidepot)  // all player bets are the same, end here
			break;
		
	} while (true);

#if 0
	for (unsigned int i=0; i < pots.size(); i++)
	{
		log_msg("pot", "#%d: amount=%0.2f players=%d",
			i+1, pots[i].amount, (int)pots[i].players.size());
		
		for (unsigned int j=0; j < pots[i].players.size(); j++)
		{
			Player *p = pots[i].players[j];
			log_msg("pot", "    player %d", p->getClientId());
		}
	}
	
	for (unsigned int i=0; i < seats.size(); i++)
	{
		log_msg("seat-bets", "seat-%d: %.2f", i, seats[i].bet);
	}
#endif
}

void Table::resetLastPlayerActions()
{
	// reset last-player action
	for (unsigned int i = 0; i < 10; i++)
	{
		if (!seats[i].occupied)
			continue;
		
		seats[i].player->resetLastAction();
	}
}

void Table::scheduleState(State sched_state, unsigned int delay_sec)
{
	state = sched_state;
	delay = delay_sec;
	delay_start = time(NULL);
}
