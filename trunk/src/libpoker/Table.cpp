
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
