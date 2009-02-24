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


#ifndef _GAME_H
#define _GAME_H

#include <vector>
#include <map>
#include <string>
#include <ctime>

#include "Config.h"
#include "Platform.h"
#include "Network.h"
#include "Protocol.h"

#include "GameController.hpp"


typedef enum {
	Connected = 0x01,
	Introduced = 0x02,
	SentInfo = 0x04,
	Authed = 0x08
} clientstate;

typedef struct {
	socktype sock;
	int id;
	//time_t lastdata;
	
	sockaddr_in saddr;
	
	char msgbuf[1024];
	int buflen;
	int last_msgid;
	
	unsigned int state;
	unsigned int version;
	char uuid[40];  // 16*2 + 4 sep + \0 = 37
	char name[64];
} clientcon;

typedef struct {
	int id;
	//sockaddr_in saddr;
	time_t logout_time;
} clientcon_archive;


typedef std::map<int,GameController*>	games_type;

typedef std::vector<clientcon>	clients_type;
typedef std::map<std::string,clientcon_archive>	clientconar_type;

// used by pserver.cpp
int gameloop();
clients_type& get_client_vector();
bool client_add(socktype sock, sockaddr_in *saddr);
bool client_remove(socktype sock);
int client_handle(socktype sock);

// used by GameController.cpp
bool client_chat(int from_gid, int from_tid, int to, const char *message);
bool client_snapshot(int from_gid, int from_tid, int to, int sid, const char *message);


#endif /* _GAME_H */
