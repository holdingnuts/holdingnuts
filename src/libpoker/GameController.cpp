#include <cstdio>

#include "Config.h"
#include "Debug.h"
#include "GameController.hpp"

#include "game.hpp"


using namespace std;

extern clientcon* get_client_by_sock(socktype sock);
//extern bool client_chat(int from, int to, const char *msg);
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

bool GameController::setPlayerMax(unsigned int max)
{
	if (max < 2 || max > MAX_PLAYERS)
		return false;
	
	max_players = max;
	return true;
}

void GameController::chatTable(int tid, const char* msg)
{
	for (vector<Player>::iterator e = players.begin(); e != players.end(); e++)
		client_chat(game_id, tid, e->client_id, msg);
}

void GameController::tick()
{
	if (!started)
	{
		if (getPlayerCount() == max_players)
		{
			dbg_print("game", "game %d has been started", game_id);
			
			char msg[1024];
			snprintf(msg, sizeof(msg), "game %d has been started", game_id);
			chatTable(-1, msg);
			
			started = true;
		}
	}
}
