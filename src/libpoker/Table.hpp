#ifndef _TABLE_H
#define _TABLE_H

#include "CommunityCards.hpp"

class Table
{
friend class GameController;

public:
	Table();
	
private:
	int table_id;
	CommunityCards communitycards;
};


#endif /* _TABLE_H */
