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
	if (pos + 1 == seats.size())
		return 0;
	else
		return pos + 1;
}

int Table::getNextActivePlayer(unsigned int pos)
{
	unsigned int start = pos;
	unsigned int cur = pos;
	bool found = false;
	
	do
	{
		cur = getNextPlayer(cur);
		
		if (seats[cur].in_round)
			found = true;
		
		// no active player left
		if (start == cur)
			return -1;
	} while (!found);
	
	return cur;
}

unsigned int Table::countActivePlayers()
{
	unsigned int count = 0;
	
	for (unsigned int i=0; i < seats.size(); i++)
	{
		if (seats[i].in_round)
			count++;
	}
	
	return count;
}
