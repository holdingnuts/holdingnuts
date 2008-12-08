
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
	unsigned int cur = cur_player;
	unsigned int start = cur;
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
