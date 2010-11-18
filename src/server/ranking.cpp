/*
 * Copyright 2008-2010, Dominik Geyer
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


#include <cmath>

#include "Config.h"
#include "Platform.h"
#include "Debug.h"
#include "Logger.h"
#include "Database.hpp"

#include "game.hpp"
#include "Player.hpp"

#include "ranking.hpp"


#ifndef NOSQLITE
extern Database *db;

/*
Example:
- Score is 500 (max=1000, min=1)
- 9 players, 3rd place
==================================
 9   8   7   6   5   4  [3]  2   1      // place
-  -  -  -  -  -  -  -  -  -  -  -
 1   2   3   4   5   6  [7]  8   9      // inverted place  (1=#9 and 9=#1)
--+---+---+---+---+---+---+---+---
-4  -3  -2  -1   0   1  [2]  3   4      // result
==================================
<-- neg.  -->    N   <--  pos. -->      // -4 to -1 negative; 0 neutral; 1 to 4 positive
                     {- divisor -}

(( ratio=0.50 | diff=15 | inv_place=7 | offset=5 divisor=4 | result=2 ))


inv_place := 9 - 3 + 1                 // = 7
offset := (9 + 1) / 2                  // = 5
divisor := 5 - 1                       // = 4.0
result := 7 - 5                        // = 2
ratio := 500 / 1000                    // = 0.5
max_diff := 1000 * 0.125               // = 125
tmp_score := (125 * 0.5) * (2 / 4.0)   // = 31
score := 500 + 31                      // = 531



Example:
- Score is 800 (max=1000, min=1)
- 8 players, 8th place
==============================
 [8] 7   6   5   4   3   2   1      // place
  -  -  -  -  -  -  -  -  -  -
 [1] 2   3   4   5   6   7   8      // inverted place  (1=#8 and 8=#1)
--+---+---+---+---+---+---+---
[-4]-3  -2  -1   0   1   2   3      // result
==============================
<--neg.-->   N   <--  pos. -->      // -4 to -1 negative; 0 neutral; 1 to 3 positive
{-  divisor  -}     {-divisor-}
div. for lose       div. for win


old_score=800
ratio=0.80 | diff=-133 | inv_place=1 | o=5 d=3 | result=-4
new_score=667

*/


static unsigned int calc_score(unsigned int score, unsigned int num_players, unsigned int place)
{
	const unsigned int min_score = 1;
	const unsigned int max_score = 1000;
	
	const float diff_ratio = 1 / 8.0f;
	
	// invert place value (e.g. 10 players => 1 = #10 and 10 = #1)
	const int inv_place = num_players - place + 1;

	
	int result;
	int offset = 0, divisor;
	
	if (num_players == 2)	// special case
	{
		result = (inv_place == 1) ? -1 : 1;
		divisor = 1;
	}
	else
	{
		if (num_players % 2)  // is odd
		{
			offset = (num_players + 1) / 2;
			divisor = offset - 1;
		}
		else
		{
			offset = (num_players / 2) + 1;
			
			// case 1 (win):   -3 -2 -1  0 [1  2]
			// case 2 (lose): [-3 -2 -1] 0  1  2
			divisor = (inv_place - offset > 0) ? (num_players / 2) - 1 : num_players / 2;
		}
		
		result = inv_place - offset;
	}
	
	
	// ratio of score to max_score
	float ratio = ((float)score / (float)max_score);
	
	// on win invert ratio => win_ratio
	if (result > 0)
		ratio = 1.0f - ratio;
	
	// prevent ratio getting too small
	if (ratio < 0.01f)
		ratio = 0.01f;
	
	// max. value to add/substract
	const float max_diff = max_score * diff_ratio;
	
	// actual score to add/substract
	int tmp_score = (int) ceil((max_diff * ratio) * (result / (float)divisor));
		
	dbg_msg("score", "old_score=%d | ratio=%.2f | diff=%d | inv_place=%d | o=%d d=%d | result=%d | new_score=%d",
		score, ratio, tmp_score, inv_place, offset, divisor, result, score + tmp_score);
	
	
	// update score
	score += tmp_score;

	
	// check if score within margin
	if (score > max_score)
		score = max_score;
	else if (score < min_score)
		score = min_score;

	return score;
}


void ranking_update(const GameController *g)
{
	std::vector<Player*> player_list;
	g->getFinishList(player_list);
	
	bool query_error = false;
	int rc = db->query("BEGIN TRANSACTION;");
	
	for (unsigned int u=0; u < player_list.size(); u++)
	{
		const Player* player = player_list[u];
		const std::string &uuid = player->getPlayerUUID();
		const int place = g->getPlayerCount() - u;
		
		dbg_msg("finish", "%s finished @ #%d",
			uuid.c_str(), place);
		
		// skip empty UUID
		if (!uuid.length())
			continue;
		
		QueryResult *result;
		rc = db->query(&result, "SELECT ranking FROM players WHERE uuid = '%q' LIMIT 1;", uuid.c_str());
		
		if (rc)
		{
			query_error = true;
			db->freeQueryResult(&result);
			break;
		}
		else
		{
			const clientcon *client = get_client_by_id(player->getClientId());
			std::string player_name = "__unknown__";
			
			// use either stored or current player name
			if (client)
				player_name = client->info.name;
			
			
			if (!result->numRows())	// insert new row
			{
				dbg_msg("SQL", "no rows, inserting...");
				
				const int initial_score = 500;
				rc = db->query("INSERT INTO players "
					"(uuid,name,t_lastgame,gamecount,ranking) VALUES('%q','%q',datetime('now'),%d,%d);",
						uuid.c_str(), player_name.c_str(),
						1, calc_score(initial_score, g->getPlayerCount(), place));
				
				if (rc)
				{
					query_error = true;
					db->freeQueryResult(&result);
					break;
				}
			}
			else		// update row
			{
				const int old_score = atoi(result->getRow(0,0));
				const int new_score = calc_score(old_score, g->getPlayerCount(), place);
				
				dbg_msg("RATING", "uuid=%s  old_score=%d  new_score=%d",
					uuid.c_str(), old_score, new_score);
				
				// update player name only if client is connected
				char *q_setname = 0;	// Note: mingw32 compiler warning if not set to 0
				if (client)
					q_setname = db->createQueryString("name = '%q',", player_name.c_str());
				
				rc = db->query("UPDATE players "
					"SET %s t_lastgame = datetime('now'), gamecount = gamecount + 1, ranking = %d "
					"WHERE uuid = '%q';", (client) ? q_setname : "", new_score, uuid.c_str());
				
				if (client)
					db->freeQueryString(q_setname);
				
				if (rc)
				{
					query_error = true;
					db->freeQueryResult(&result);
					break;
				}
			}
		}
		
		db->freeQueryResult(&result);
	}
	
	if (!query_error)
		rc = db->query("COMMIT;");
	else
	{
		rc = db->query("ROLLBACK;");
		log_msg("update_scores", "There was an error during transaction. Rolling back.");
	}
}


void ranking_setup()
{
	/* create players table if not already exists */
	db->query("CREATE TABLE IF NOT EXISTS players "
		"(uuid varchar(50) NOT NULL PRIMARY KEY, name varchar(50), "
		"t_lastgame DATE NOT NULL, gamecount INT NOT NULL, ranking INT NOT NULL);");
}

#endif /* !NOSQLITE */

