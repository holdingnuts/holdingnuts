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
#include <cstdlib>
#include <cstring>

#include "Config.h"
#include "Platform.h"
#include "Network.h"
#include "Debug.h"
#include "Logger.h"
#include "Tokenizer.hpp"
#include "ConfigParser.hpp"

#include "game.hpp"


using namespace std;

extern ConfigParser config;

// temporary buffer for sending messages
#define MSG_BUFFER_SIZE  (1024*16)
static char msg[MSG_BUFFER_SIZE];

static games_type games;
static unsigned int gid_counter = 0;

static clients_type clients;
static unsigned int cid_counter = 0;


static clientconar_type con_archive;
static time_t last_conarchive_cleanup = 0;   // last time scan


GameController* get_game_by_id(int gid)
{
	games_type::const_iterator it = games.find(gid);
	if (it != games.end())
		return it->second;
	else
		return NULL;
}

// for pserver.cpp filling FD_SET
clients_type& get_client_vector()
{
	return clients;
}

clientcon* get_client_by_sock(socktype sock)
{
	for (unsigned int i=0; i < clients.size(); i++)
		if (clients[i].sock == sock)
			return &(clients[i]);
	
	return NULL;
}

clientcon* get_client_by_id(int cid)
{
	for (unsigned int i=0; i < clients.size(); i++)
		if (clients[i].id == cid)
			return &(clients[i]);
	
	return NULL;
}

int send_msg(socktype sock, const char *message)
{
	char buf[MSG_BUFFER_SIZE];
	const int len = snprintf(buf, sizeof(buf), "%s\r\n", message);
	const int bytes = socket_write(sock, buf, len);
	
	// FIXME: send remaining bytes if not all have been sent
	if (len != bytes)
		log_msg("clientsock", "(%d) warning: not all bytes written (%d != %d)", sock, len, bytes);
	
	return bytes;
}

bool send_response(socktype sock, bool is_success, int last_msgid, int code=0, const char *str="")
{
	char buf[512];
	if (last_msgid == -1)
		snprintf(buf, sizeof(buf), "%s %d %s",
			is_success ? "OK" : "ERR", code, str);
	else
		snprintf(buf, sizeof(buf), "%d %s %d %s",
			  last_msgid, is_success ? "OK" : "ERR", code, str);
	
	return send_msg(sock, buf);
}

bool send_ok(clientcon *client, int code=0, const char *str="")
{
#if 0
	return send_response(client->sock, true, client->last_msgid, code, str);
#else
	return true;
#endif
}

bool send_err(clientcon *client, int code=0, const char *str="")
{
	return send_response(client->sock, false, client->last_msgid, code, str);
}

// from client/foyer to client/foyer
bool client_chat(int from, int to, const char *message)
{
	char msg[256];
	
	if (from == -1)
	{
		snprintf(msg, sizeof(msg), "MSG %d %s %s",
			from, "foyer", message);
	}
	else
	{
		clientcon* fromclient = get_client_by_id(from);
		
		snprintf(msg, sizeof(msg), "MSG %d \"%s\" %s",
			from,
			(fromclient) ? fromclient->info.name : "???",
			message);
	}
	
	if (to == -1)
	{
		for (clients_type::iterator e = clients.begin(); e != clients.end(); e++)
		{
			if (!(e->state & Introduced))  // do not send broadcast to non-introduced clients
				continue;
			
			send_msg(e->sock, msg);
		}
	}
	else
	{
		clientcon* toclient = get_client_by_id(to);
		if (toclient)
			send_msg(toclient->sock, msg);
		else
			return false;
	}
	
	return true;
}

// from game/table to client
bool client_chat(int from_gid, int from_tid, int to, const char *message)
{
	char msg[256];
	
	snprintf(msg, sizeof(msg), "MSG %d:%d %s %s",
		from_gid, from_tid, (from_tid == -1) ? "game" : "table", message);
	
	clientcon* toclient = get_client_by_id(to);
	if (toclient)
		send_msg(toclient->sock, msg);
	
	return true;
}

// from client to game/table
bool table_chat(int from_cid, int to_gid, int to_tid, const char *message)
{
	char msg[256];
	
	clientcon* fromclient = get_client_by_id(from_cid);
	
	GameController *g = get_game_by_id(to_gid);
	if (!g)
		return false;
	
	vector<int> client_list;
	g->getListenerList(client_list);
	
	for (unsigned int i=0; i < client_list.size(); i++)
	{
		snprintf(msg, sizeof(msg), "MSG %d:%d:%d \"%s\" %s",
			to_gid, to_tid, from_cid,
			(fromclient) ? fromclient->info.name : "???",
			message);
		
		clientcon* toclient = get_client_by_id(client_list[i]);
		if (toclient)
			send_msg(toclient->sock, msg);
	}
	
	return true;
}

bool client_snapshot(int from_gid, int from_tid, int to, int sid, const char *message)
{
	char buf[MSG_BUFFER_SIZE];
	snprintf(buf, sizeof(buf), "SNAP %d:%d %d %s",
		from_gid, from_tid, sid, message);
	
	clientcon* toclient = get_client_by_id(to);
	if (toclient && toclient->state & Introduced)
		send_msg(toclient->sock, buf);
	
	return true;
}

bool client_snapshot(int to, int sid, const char *message)
{
	if (to == -1)  // to all
	{
		for (clients_type::iterator e = clients.begin(); e != clients.end(); e++)
			client_snapshot(-1, -1, e->id, sid, message);
	}
	else
		client_snapshot(-1, -1, to, sid, message);
	
	return true;
}

bool client_add(socktype sock, sockaddr_in *saddr)
{
	// drop client if maximum connection count is reached
	if (clients.size() == SERVER_CLIENT_HARDLIMIT || clients.size() == (unsigned int) config.getInt("max_clients"))
	{
		send_response(sock, false, -1, ErrServerFull, "server full");
		socket_close(sock);
		
		return false;
	}
	
	
	// drop client if maximum connections per IP is reached
	const unsigned int connection_max = (unsigned int) config.getInt("max_connections_per_ip");
	if (connection_max)
	{
		unsigned int connection_count = 0;
		for (clients_type::const_iterator client = clients.begin(); client != clients.end(); client++)
		{
			// does the IP match?
			if (!memcmp(&client->saddr.sin_addr, &saddr->sin_addr, sizeof(saddr->sin_addr)))
			{
				if (++connection_count == connection_max)
				{
					send_response(sock, false, -1, ErrMaxConnectionsPerIP, "connection limit per IP is reached");
					socket_close(sock);
					
					return false;
				}
			}
		}
	}
	
	// add the client
	clientcon client;
	memset(&client, 0, sizeof(client));
	client.sock = sock;
	client.saddr = *saddr;
	client.id = -1;
	
	// set initial state
	client.state |= Connected;
	
	clients.push_back(client);
	
	return true;
}

bool client_remove(socktype sock)
{
	for (clients_type::iterator client = clients.begin(); client != clients.end(); client++)
	{
		if (client->sock == sock)
		{
			socket_close(client->sock);
			
			bool send_msg = false;
			if (client->state & SentInfo)
			{
				// remove player from unstarted games
				for (games_type::iterator e = games.begin(); e != games.end(); e++)
				{
					GameController *g = e->second;
					if (!g->isStarted() && g->isPlayer(client->id))
						g->removePlayer(client->id);
				}
				
				
				snprintf(msg, sizeof(msg),
					"%d %d \"%s\"",
					SnapFoyerLeave, client->id, client->info.name);
				
				send_msg = true;
				
				// save client-con in archive
				string uuid = client->uuid;
				
				if (uuid.length())
				{
					// FIXME: only add max. 3 entries for each IP
					con_archive[uuid].logout_time = time(NULL);
				}
			}
			
			log_msg("clientsock", "(%d) connection closed", client->sock);
			
			clients.erase(client);
			
			// send foyer snapshot to all remaining clients
			if (send_msg)
				client_snapshot(-1, SnapFoyer, msg);
			
			break;
		}
	}
	
	return true;
}

int client_cmd_pclient(clientcon *client, Tokenizer &t)
{
	unsigned int version = t.getNextInt();
	string uuid = t.getNext();
	
	if (version < VERSION_COMPAT)
	{
		log_msg("client", "client %d version (%d) too old", client->sock, version);
		send_err(client, ErrWrongVersion, "The client version is too old."
			"Please update your HoldingNuts client to a more recent version.");
		client_remove(client->sock);
	}
	else
	{
		// ack the command
		send_ok(client);
		
		
		client->version = version;
		client->state |= Introduced;
		
		snprintf(client->uuid, sizeof(client->uuid), "%s", uuid.c_str());
		
		// re-assign cid if this client was previously connected (and cid isn't already connected)
		bool use_prev_cid = false;
		
		if (uuid.length())
		{
			clientconar_type::iterator it = con_archive.find(uuid);
			
			if (it != con_archive.end())
			{
				clientcon *conc = get_client_by_id(it->second.id);
				if (!conc)
				{
					client->id = it->second.id;
					use_prev_cid = true;
					
					log_msg("uuid", "(%d) using previous cid (%d) for uuid '%s'", client->sock, client->id, client->uuid);
				}
				else
				{
					log_msg("uuid", "(%d) uuid '%s' already connected; used by cid %d", client->sock, client->uuid, conc->id);
					client->uuid[0] = '\0';    // client is not allowed to use this uuid
				}
			}
			else
				log_msg("uuid", "(%d) reserving uuid '%s'", client->sock, client->uuid);
		}
		
		if (!use_prev_cid)
			client->id = cid_counter++;
		
		
		// set initial client info
		snprintf(client->info.name, sizeof(client->info.name), "client_%d", client->id);
		*(client->info.location) = '\0';
		
		// send 'introduced response'
		snprintf(msg, sizeof(msg), "PSERVER %d %d %d",
			VERSION,
			client->id,
			(unsigned int) time(NULL));
			
		send_msg(client->sock, msg);
	}
	
	return 0;
}

int client_cmd_info(clientcon *client, Tokenizer &t)
{
	string infostr;
	Tokenizer it(":");
	
	while (t.getNext(infostr))
	{
		it.parse(infostr);
		
		string infotype, infoarg;
		it.getNext(infotype);
		
		bool havearg = it.getNext(infoarg);
		
		if (infotype == "name" && havearg)
		{
			// allow name-change only once per session
			if (!(client->state & SentInfo))
				snprintf(client->info.name, sizeof(client->info.name), "%s", infoarg.c_str());
		}
		else if (infotype == "location" && havearg)
			snprintf(client->info.location, sizeof(client->info.location), "%s", infoarg.c_str());
	}
	
	send_ok(client);
	
	if (!(client->state & SentInfo))
	{
		// store UUID in connection-archive
		if (*client->uuid)
		{
			clientcon_archive ar;
			memset(&ar, 0, sizeof(ar));
			ar.id = client->id;
			con_archive[client->uuid] = ar;
		}
		
		
		// send welcome message
		const string welcome_message = config.get("welcome_message");
		if (welcome_message.length())
		{
			snprintf(msg, sizeof(msg),
				"%s",
				welcome_message.c_str());
		
			client_chat(-1, client->id, msg);
		}
		
		
		// send foyer snapshot broadcast
		snprintf(msg, sizeof(msg),
			"%d %d \"%s\"",
			SnapFoyerJoin, client->id, client->info.name);
		
		client_snapshot(-1, SnapFoyer, msg);
	}
	
	client->state |= SentInfo;
	
	return 0;
}

int client_cmd_chat(clientcon *client, Tokenizer &t)
{
	bool cmderr = false;
	
	if (t.count() < 2)
		cmderr = true;
	else
	{
		// flooding-protection
		int time_since_last_chat = (int) difftime(time(NULL), client->last_chat);
		
		// is the client still muted?
		if (time_since_last_chat < 0)
		{
			send_err(client, 0, "you are still muted");
			return 0;
		}
		
		if ((unsigned int)time_since_last_chat > (unsigned int) config.getInt("flood_chat_interval"))
		{
			// reset flood-measure for new interval
			client->last_chat = time(NULL);
			client->chat_count = 0;
		}
		
		// is client flooding?
		if (++client->chat_count >= (unsigned int) config.getInt("flood_chat_per_interval"))
		{
			log_msg("flooding", "client (%d) caught flooding the chat", client->id);
			
			// mute client for n-seconds
			client->last_chat = time(NULL) + config.getInt("flood_chat_mute");
			client->chat_count = 0;
			
			send_err(client, 0, "you have been muted for some time");
			return 0;
		}
		
		
		Tokenizer ct(":");
		ct.parse(t.getNext());
		string chatmsg = t.getTillEnd();
		
		if (ct.count() == 1) // cid
		{
			int dest = ct.getNextInt();
			
			if (!client_chat(client->id, dest, chatmsg.c_str()))
				cmderr = true;
		}
		else if (ct.count() == 2)  // gid:tid
		{
			int gid = ct.getNextInt();
			int tid = ct.getNextInt();
			
			if (!table_chat(client->id, gid, tid, chatmsg.c_str()))
				cmderr = true;
		}
	}
	
	if (!cmderr)
		send_ok(client);
	else
		send_err(client);
	
	return 0;
}


bool send_gameinfo(clientcon *client, int gid)
{
	const GameController *g;
	if (!(g = get_game_by_id(gid)))
		return false;
	
	int game_mode = 0;
	switch ((int)g->getGameType())
	{
	case GameController::SNG:
		game_mode = GameModeSNG;
		break;
	case GameController::FreezeOut:
		game_mode = GameModeFreezeOut;
		break;
	case GameController::RingGame:
		game_mode = GameModeRingGame;
		break;
	}
	
	int state = 0;
	if (g->isEnded())
		state = GameStateEnded;
	else if (g->isStarted())
		state = GameStateStarted;
	else
		state = GameStateWaiting;
	
	snprintf(msg, sizeof(msg),
		"GAMEINFO %d %d:%d:%d:%d:%d:%d:%d:%d %d:%d:%d \"%s\"",
		gid,
		(int) GameTypeHoldem,
		game_mode,
		state,
		(g->isPlayer(client->id) ? GameInfoRegistered : 0) |
			(g->isSpectator(client->id) ? GameInfoSubscribed : 0) |
			(g->hasPassword() ? GameInfoPassword : 0) |
			(g->getOwner() == client->id ? GameInfoOwner : 0) |
			(g->getRestart() ? GameInfoRestart : 0),
		g->getPlayerMax(),
		g->getPlayerCount(),
		g->getPlayerTimeout(),
		g->getPlayerStakes(),
		g->getBlindsStart(),
		int(g->getBlindsFactor() * 10),
		g->getBlindsTime(),
		g->getName().c_str());
	
	send_msg(client->sock, msg);
	
	return true;
}

bool client_cmd_request_gameinfo(clientcon *client, Tokenizer &t)
{
	string sgid;
	while (t.getNext(sgid))   // FIXME: have maximum for count of requests
	{
		const int gid = Tokenizer::string2int(sgid);
		send_gameinfo(client, gid);
	}
	
	return true;
}

bool client_cmd_request_clientinfo(clientcon *client, Tokenizer &t)
{
	string scid;
	while (t.getNext(scid))   // FIXME: have maximum for count of requests
	{
		const socktype cid = Tokenizer::string2int(scid);
		const clientcon *c;
		if ((c = get_client_by_id(cid)))
		{
			snprintf(msg, sizeof(msg),
				"CLIENTINFO %d \"name:%s\" \"location:%s\"",
				cid,
				c->info.name, c->info.location);
			
			send_msg(client->sock, msg);
		}
	}
	
	return true;
}

bool client_cmd_request_gamelist(clientcon *client, Tokenizer &t)
{
	string gamelist;
	for (games_type::iterator e = games.begin(); e != games.end(); e++)
	{
		const int gid = e->first;
		
		snprintf(msg, sizeof(msg), "%d ", gid);
		gamelist += msg;
	}
	
	snprintf(msg, sizeof(msg),
		"GAMELIST %s", gamelist.c_str());
	
	send_msg(client->sock, msg);
	
	return true;
}

bool client_cmd_request_playerlist(clientcon *client, Tokenizer &t)
{
	int gid;
	t >> gid;
	
	GameController *g = get_game_by_id(gid);
	if (!g)
		return false;
	
	vector<int> client_list;
	g->getPlayerList(client_list);
	
	string slist;
	for (unsigned int i=0; i < client_list.size(); i++)
	{
		snprintf(msg, sizeof(msg), "%d ", client_list[i]);
		slist += msg;
	}
	
	snprintf(msg, sizeof(msg), "PLAYERLIST %d %s", gid, slist.c_str());
	send_msg(client->sock, msg);
	
	return true;
}

bool client_cmd_request_serverinfo(clientcon *client, Tokenizer &t)
{
	snprintf(msg, sizeof(msg), "SERVERINFO %d:%d:%d",
		(int) clients.size(),
		(int) con_archive.size(),
		(int) games.size());
	
	send_msg(client->sock, msg);
	
	return true;
}

bool client_cmd_request_gamestart(clientcon *client, Tokenizer &t)
{
	int gid;
	t >> gid;
	
	GameController *g = get_game_by_id(gid);
	if (!g)
		return false;
	
	if (g->getOwner() != client->id && !(client->state & Authed))
		return false;
	
	g->start();
	
	return true;
}

bool client_cmd_request_gamerestart(clientcon *client, Tokenizer &t)
{
	int gid, restart;
	t >> gid >> restart;

	GameController *g = get_game_by_id(gid);
	if (!g)
		return false;

	if (!(client->state & Authed))
		return false;

	g->setRestart(restart);

	return true;
}

int client_cmd_request(clientcon *client, Tokenizer &t)
{
	if (!t.count())
	{
		send_err(client, ErrParameters);
		return 1;
	}
	
	bool cmderr = false;
	
	string request;
	t >> request;
	
	if (request == "clientinfo")
		cmderr = !client_cmd_request_clientinfo(client, t);
	else if (request == "gameinfo")
		cmderr = !client_cmd_request_gameinfo(client, t);
	else if (request == "gamelist")
		cmderr = !client_cmd_request_gamelist(client, t);
	else if (request == "playerlist")
		cmderr = !client_cmd_request_playerlist(client, t);
	else if (request == "serverinfo")
		cmderr = !client_cmd_request_serverinfo(client, t);
	else if (request == "start")
		cmderr = !client_cmd_request_gamestart(client, t);
	else if (request == "restart")
		cmderr = !client_cmd_request_gamerestart(client, t);
	else
		cmderr = true;
	
	// FIXME: temporarily disabled for release 0.0.3
#if 0
	if (!cmderr)
		send_ok(client);
	else
		send_err(client);
#endif
	
	return 0;
}

int client_cmd_register(clientcon *client, Tokenizer &t)
{
	if (!t.count())
	{
		send_err(client, ErrParameters);
		return 1;
	}
	
	int gid;
	t >> gid;
	
	string passwd = "";
	if (t.count() >=2)
		t >> passwd;
	
	GameController *g = get_game_by_id(gid);
	if (!g)
	{
		send_err(client, 0 /*FIXME*/, "game does not exist");
		return 1;
	}
	
	if (g->isStarted())
	{
		send_err(client, 0 /*FIXME*/, "game has already been started");
		return 1;
	}
	
	if (g->isPlayer(client->id))
	{
		send_err(client, 0 /*FIXME*/, "you are already registered");
		return 1;
	}
	
	// check for max-games-register limit
	const unsigned int register_limit = config.getInt("max_register_per_player");
	unsigned int count = 0;
	for (games_type::const_iterator e = games.begin(); e != games.end(); e++)
	{
		GameController *g = e->second;
		
		if (g->isPlayer(client->id))
		{
			if (++count == register_limit)
			{
				send_err(client, 0 /*FIXME*/, "register limit per player is reached");
				return 1;
			}
		}
	}
	
	// check the password
	if (!g->checkPassword(passwd))
	{
		send_err(client, 0 /*FIXME*/, "unable to register, wrong password");
		return 1;
	}
	
	if (!g->addPlayer(client->id))
	{
		send_err(client, 0 /*FIXME*/, "unable to register");
		return 1;
	}
	
	
	log_msg("game", "%s (%d) joined game %d (%d/%d)",
		client->info.name, client->id, gid,
		g->getPlayerCount(), g->getPlayerMax());
	
	
	send_ok(client);
	
	return 0;
}

int client_cmd_unregister(clientcon *client, Tokenizer &t)
{
	if (!t.count())
	{
		send_err(client, ErrParameters);
		return 1;
	}
	
	int gid;
	t >> gid;
	
	GameController *g = get_game_by_id(gid);
	if (!g)
	{
		send_err(client, 0 /*FIXME*/, "game does not exist");
		return 1;
	}
	
	if (g->isStarted())
	{
		send_err(client, 0 /*FIXME*/, "game has already been started");
		return 1;
	}
	
	if (!g->isPlayer(client->id))
	{
		send_err(client, 0 /*FIXME*/, "you are not registered");
		return 1;
	}
	
	if (!g->removePlayer(client->id))
	{
		send_err(client, 0 /*FIXME*/, "unable to unregister");
		return 1;
	}
	
	
	log_msg("game", "%s (%d) parted game %d (%d/%d)",
		client->info.name, client->id, gid,
		g->getPlayerCount(), g->getPlayerMax());
	
	
	send_ok(client);
	
	return 0;
}

int client_cmd_subscribe(clientcon *client, Tokenizer &t)
{
	if (!t.count())
	{
		send_err(client, ErrParameters);
		return 1;
	}
	
	int gid;
	t >> gid;
	
	string passwd = "";
	if (t.count() >=2)
		t >> passwd;
	
	GameController *g = get_game_by_id(gid);
	if (!g)
	{
		send_err(client, 0 /*FIXME*/, "game does not exist");
		return 1;
	}
	
	if (g->isSpectator(client->id))
	{
		send_err(client, 0 /*FIXME*/, "you are already subscribed");
		return 1;
	}
	
	// check for max-games-subscribe limit
	const unsigned int subscribe_limit = config.getInt("max_subscribe_per_player");
	unsigned int count = 0;
	for (games_type::const_iterator e = games.begin(); e != games.end(); e++)
	{
		GameController *g = e->second;
		
		if (g->isSpectator(client->id))
		{
			if (++count == subscribe_limit)
			{
				send_err(client, 0 /*FIXME*/, "subscribe limit per player is reached");
				return 1;
			}
		}
	}

	// check the password
	if (!g->checkPassword(passwd))
	{
		send_err(client, 0 /*FIXME*/, "unable to subscribe, wrong password");
		return 1;
	}
	
	if (!g->addSpectator(client->id))
	{
		send_err(client, 0 /*FIXME*/, "unable to subscribe");
		return 1;
	}
	
	
	log_msg("game", "%s (%d) subscribed game %d",
		client->info.name, client->id, gid);
	
	
	send_ok(client);
	
	return 0;
}

int client_cmd_unsubscribe(clientcon *client, Tokenizer &t)
{
	if (!t.count())
	{
		send_err(client, ErrParameters);
		return 1;
	}
	
	int gid;
	t >> gid;
	
	GameController *g = get_game_by_id(gid);
	if (!g)
	{
		send_err(client, 0 /*FIXME*/, "game does not exist");
		return 1;
	}
	
	if (!g->isSpectator(client->id))
	{
		send_err(client, 0 /*FIXME*/, "you are not subscribed");
		return 1;
	}
	
	if (!g->removeSpectator(client->id))
	{
		send_err(client, 0 /*FIXME*/, "unable to unsubscribe");
		return 1;
	}
	
	
	log_msg("game", "%s (%d) unsubscribed game %d",
		client->info.name, client->id, gid);
	
	
	send_ok(client);
	
	return 0;
}

int client_cmd_action(clientcon *client, Tokenizer &t)
{
	if (t.count() < 2)
	{
		send_err(client, ErrParameters);
		return 1;
	}
	
	int gid;
	string action;
	chips_type amount;
	
	t >> gid >> action;
	amount = t.getNextInt();
	
	GameController *g = get_game_by_id(gid);
	if (!g)
	{
		send_err(client, 0 /* FIXME */, "game does not exist");
		return 1;
	}
	
	Player::PlayerAction a = Player::None;
	
	if (action == "check")
		a = Player::Check;
	else if (action == "fold")
		a = Player::Fold;
	else if (action == "call")
		a = Player::Call;
	else if (action == "bet")
		a = Player::Bet;
	else if (action == "raise")
		a = Player::Raise;
	else if (action == "allin")
		a = Player::Allin;
	else if (action == "show")
		a = Player::Show;
	else if (action == "muck")
		a = Player::Muck;
	else if (action == "sitout")
		a = Player::Sitout;
	else if (action == "back")
		a = Player::Back;
	else if (action == "reset")
		a = Player::ResetAction;
	else
	{
		send_err(client, ErrParameters);
		return 1;
	}
	
	
	g->setPlayerAction(client->id, a, amount);
	
	send_ok(client);
	
	return 0;
}

int client_cmd_create(clientcon *client, Tokenizer &t)
{
	if (!config.getBool("perm_create_user") && !(client->state & Authed))
	{
		send_err(client, ErrNoPermission, "no permission");
		return 1;
	}
	
	
	// check for server games count limit
	if (games.size() >= (unsigned int) config.getInt("max_games"))
	{
		send_err(client, 0 /*FIXME*/, "server games count reached");
		return 1;
	}
	
	
	// check for max-games-create limit
	unsigned int create_limit = config.getInt("max_create_per_player");
	unsigned int count = 0;
	for (games_type::const_iterator e = games.begin(); e != games.end(); e++)
	{
		GameController *g = e->second;
		
		if (g->getOwner() == client->id)
		{
			if (++count == create_limit)
			{
				send_err(client, 0 /*FIXME*/, "create limit per player is reached");
				return 1;
			}
		}
	}
	
	
	bool cmderr = false;
	
	struct {
		string name;
		unsigned int max_players;
		int type;
		chips_type stake;
		unsigned int timeout;
		chips_type blinds_start;
		float blinds_factor;
		unsigned int blinds_time;
		string password;
		bool restart;
	} ginfo = {
		"user_game",
		10,
		GameController::SNG,
		1500,
		30,
		20,
		2.0f,
		180,
		"",
		false,
	};
	
	
	string infostr;
	Tokenizer it(":");
	
	while (t.getNext(infostr))
	{
		it.parse(infostr);
		
		string infotype, infoarg;
		it.getNext(infotype);
		
		bool havearg = it.getNext(infoarg);
		
		if (infotype == "type" && havearg)
		{
			ginfo.type = Tokenizer::string2int(infoarg);
			
			// TODO: no other gamesmodes supported yet
			if (ginfo.type != GameController::SNG)
				cmderr = true;
		}
		else if (infotype == "players" && havearg)
		{
			ginfo.max_players = Tokenizer::string2int(infoarg);
			
			if (ginfo.max_players < 2 || ginfo.max_players > 10)
				cmderr = true;
		}
		else if (infotype == "stake" && havearg)
		{
			ginfo.stake = Tokenizer::string2int(infoarg);
			
			if (ginfo.stake < 10 || ginfo.stake > 1000000*100)
				cmderr = true;
		}
		else if (infotype == "timeout" && havearg)
		{
			ginfo.timeout = Tokenizer::string2int(infoarg);
			
			if (ginfo.timeout < 5 || ginfo.timeout > 5*60)
				cmderr = true;
		}
		else if (infotype == "name" && havearg)
		{
			if (infoarg.length() > 50)
				infoarg = string(infoarg, 0, 50);
			
			ginfo.name = infoarg;
		}
		else if (infotype == "blinds_start" && havearg)
		{
			ginfo.blinds_start = Tokenizer::string2int(infoarg);
			
			if (ginfo.blinds_start < 5 || ginfo.blinds_start > 200*100)
				cmderr = true;
		}
		else if (infotype == "blinds_factor" && havearg)
		{
			ginfo.blinds_factor = Tokenizer::string2int(infoarg) / 10.0f;
			
			if (ginfo.blinds_factor < 1.2f || ginfo.blinds_factor > 4.0f)
				cmderr = true;
		}
		else if (infotype == "blinds_time" && havearg)
		{
			ginfo.blinds_time = Tokenizer::string2int(infoarg);
			
			if (ginfo.blinds_time < 30 || ginfo.blinds_time > 30*60)
				cmderr = true;
		}
		else if (infotype == "password" && havearg)
		{
			if (infoarg.length() > 16)
				infoarg = string(infoarg, 0, 16);
			
			ginfo.password = infoarg;
		}
		else if (infotype == "restart" && havearg)
		{
			if (client->state & Authed)
				ginfo.restart = Tokenizer::string2int(infoarg) ? 1 : 0;
			else
				cmderr = true;
		}
	}
	
	if (!cmderr)
	{
		GameController *g = new GameController();
		const int gid = ++gid_counter;
		g->setGameId(gid);
		g->setPlayerMax(ginfo.max_players);
		g->setPlayerTimeout(ginfo.timeout);
		g->setPlayerStakes(ginfo.stake);
		g->addPlayer(client->id);
		g->setOwner(client->id);
		g->setName(ginfo.name);
		g->setBlindsStart(ginfo.blinds_start);
		g->setBlindsFactor(ginfo.blinds_factor);
		g->setBlindsTime(ginfo.blinds_time);
		g->setPassword(ginfo.password);
		g->setRestart(ginfo.restart);
		games[gid] = g;
		
		send_ok(client);
		
		
		log_msg("game", "%s (%d) created game %d",
			client->info.name, client->id, gid);
		
		
		for (clients_type::iterator e = clients.begin(); e != clients.end(); e++)
		{
			clientcon *client = &(*e);
			if (!(client->state & Introduced))  // do not send broadcast to non-introduced clients
				continue;
			
			send_gameinfo(client, gid);
		}
	}
	else
		send_err(client);
	
	return 0;
}

int client_cmd_auth(clientcon *client, Tokenizer &t)
{
	bool cmderr = true;
	
	if (t.count() >= 2 && config.get("auth_password").length())
	{
		const int type = t.getNextInt();
		const string passwd = t.getNext();
		
		// -1 is server-auth
		if (type == -1)
		{
			if (passwd == config.get("auth_password"))
			{
				client->state |= Authed;
				
				cmderr = false;
			}
		}
	}
	
	if (!cmderr)
	{
		log_msg("auth", "%s (%d) has been authed",
			client->info.name, client->id);
	
		send_ok(client);
	}
	else
		send_err(client, 0, "auth failed");
	
	return 0;
}

int client_cmd_config(clientcon *client, Tokenizer &t)
{
	bool cmderr = false;
	
	if (client->state & Authed)
	{
		const string action = t.getNext();
		const string varname = t.getNext();
		
		if (action == "get")
		{
			if (config.exists(varname))
				snprintf(msg, sizeof(msg), "Config: %s=%s",
					varname.c_str(),
					config.get(varname).c_str());
			else
				snprintf(msg, sizeof(msg), "Config: %s not set",
					varname.c_str());
			client_chat(-1, client->id, msg);
		}
		else if (action == "set")
		{
			const string varvalue = t.getNext();
			
			config.set(varname, varvalue);
			
			log_msg("config", "%s (%d) set var '%s' to '%s'",
				client->info.name, client->id,
				varname.c_str(), varvalue.c_str());
		}
		else
			cmderr = true;
	}
	else
		cmderr = true;
	
	if (!cmderr)
		send_ok(client);
	else
		send_err(client, 0, "config request failed");
	
	return 0;
}

int client_execute(clientcon *client, const char *cmd)
{
	Tokenizer t(" ");
	t.parse(cmd);  // parse the command line
	
	// ignore blank command
	if (!t.count())
		return 0;
	
	//dbg_msg("clientsock", "(%d) executing '%s'", client->sock, cmd);
	
	// FIXME: could be done better...
	// extract message-id if present
	const char firstchar = t[0][0];
	if (firstchar >= '0' && firstchar <= '9')
		client->last_msgid = t.getNextInt();
	else
		client->last_msgid = -1;
	
	
	// get command argument
	const string command = t.getNext();
	
	
	if (!(client->state & Introduced))  // state: not introduced
	{
		if (command == "PCLIENT")
			return client_cmd_pclient(client, t);
		else
		{
			// seems not to be a pclient
			send_err(client, ErrProtocol, "protocol error");
			return -1;
		}
	}
	else if (command == "INFO")
		return client_cmd_info(client, t);
	else if (command == "CHAT")
		return client_cmd_chat(client, t);
	else if (command == "REQUEST")
		return client_cmd_request(client, t);
	else if (command == "REGISTER")
		return client_cmd_register(client, t);
	else if (command == "UNREGISTER")
		return client_cmd_unregister(client, t);
	else if (command == "SUBSCRIBE")
		return client_cmd_subscribe(client, t);
	else if (command == "UNSUBSCRIBE")
		return client_cmd_unsubscribe(client, t);
	else if (command == "ACTION")
		return client_cmd_action(client, t);
	else if (command == "CREATE")
		return client_cmd_create(client, t);
	else if (command == "AUTH")
		return client_cmd_auth(client, t);
	else if (command == "CONFIG")
		return client_cmd_config(client, t);
	else if (command == "QUIT")
	{
		send_ok(client);
		return -1;
	}
	else
		send_err(client, ErrNotImplemented, "not implemented");
	
	return 0;
}

// returns zero if no cmd was found or no bytes remaining after exec
int client_parsebuffer(clientcon *client)
{
	//log_msg("clientsock", "(%d) parse (bufferlen=%d)", client->sock, client->buflen);
	
	int found_nl = -1;
	for (int i=0; i < client->buflen; i++)
	{
		if (client->msgbuf[i] == '\r')
			client->msgbuf[i] = ' ';  // space won't hurt
		else if (client->msgbuf[i] == '\n')
		{
			found_nl = i;
			break;
		}
	}
	
	int retval = 0;
	
	// is there a command in queue?
	if (found_nl != -1)
	{
		// extract command
		char cmd[sizeof(client->msgbuf)];
		memcpy(cmd, client->msgbuf, found_nl);
		cmd[found_nl] = '\0';
		
		//log_msg("clientsock", "(%d) command: '%s' (len=%d)", client->sock, cmd, found_nl);
		if (client_execute(client, cmd) != -1)  // client quitted ?
		{
			// move the rest to front
			memmove(client->msgbuf, client->msgbuf + found_nl + 1, client->buflen - (found_nl + 1));
			client->buflen -= found_nl + 1;
			//log_msg("clientsock", "(%d) new buffer after cmd (bufferlen=%d)", client->sock, client->buflen);
			
			retval = client->buflen;
		}
		else
		{
			client_remove(client->sock);
			retval = 0;
		}
	}
	else
		retval = 0;
	
	return retval;
}

int client_handle(socktype sock)
{
	char buf[1024];
	int bytes;
	
	// return early on client close/error
	if ((bytes = socket_read(sock, buf, sizeof(buf))) <= 0)
		return bytes;
	
	
	//log_msg("clientsock", "(%d) DATA len=%d", sock, bytes);
	
	clientcon *client = get_client_by_sock(sock);
	if (!client)
	{
		log_msg("clientsock", "(%d) error: no client associated", sock);
		return -1;
	}
	
	if (client->buflen + bytes > (int)sizeof(client->msgbuf))
	{
		log_msg("clientsock", "(%d) error: buffer size exceeded", sock);
		client->buflen = 0;
	}
	else
	{
		memcpy(client->msgbuf + client->buflen, buf, bytes);
		client->buflen += bytes;
		
		// parse and execute all commands in queue
		while (client_parsebuffer(client));
	}
	
	return bytes;
}

void remove_expired_conar_entries()
{
	time_t curtime = time(NULL);
	unsigned int expire = config.getInt("conarchive_expire");
	
#if 0
	dbg_msg("clientar", "scanning for expired entries");
#endif
	
	for (clientconar_type::iterator e = con_archive.begin(); e != con_archive.end();)
	{
		clientcon_archive *conar = &(e->second);
		
		if (conar->logout_time && (unsigned int)difftime(curtime, conar->logout_time) > expire)
		{
			dbg_msg("clientar", "removing expired entry %s", e->first.c_str());
			con_archive.erase(e++);
		}
		else
			++e;
	}
}

int gameloop()
{
#ifdef DEBUG
	// initially add games for debugging purpose
	if (!games.size())
	{
		for (int i=0; i < config.getInt("dbg_testgame_games"); i++)
		{
			GameController *g = new GameController();
			const int gid = i;
			g->setGameId(gid);
			g->setName("test game");
			g->setRestart(true);
			g->setOwner(-1);
			g->setPlayerMax(config.getInt("dbg_testgame_players"));
			g->setPlayerTimeout(config.getInt("dbg_testgame_timeout"));
			g->setPlayerStakes(config.getInt("dbg_testgame_stakes"));
			
			if (config.getBool("dbg_stresstest") && i > 10)
			{
				for (int j=0; j < config.getInt("dbg_testgame_players"); j++)
					g->addPlayer(j*1000 + i);
			}
			
			games[gid] = g;
			
			gid_counter++;
		}
	}
#endif
	
	
	// handle all games
	for (games_type::iterator e = games.begin(); e != games.end();)
	{
		GameController *g = e->second;
		
		// game has been deleted
		if (g->tick() < 0)
		{
			// replicate game if "restart" is set
			if (g->getRestart())
			{
				const int gid = ++gid_counter;
				GameController *newgame = new GameController(*g);
				
				// set new ID
				newgame->setGameId(gid);
				
				games[gid] = newgame;
				
				log_msg("game", "restarted game (old: %d, new: %d)",
					g->getGameId(), newgame->getGameId());
			}
			else
				log_msg("game", "deleting game %d", g->getGameId());
			
			delete g;
			games.erase(e++);
		}
		else
			++e;
	}
	
	
	// delete all expired archived connection-data (con_archive)
	if ((unsigned int)difftime(time(NULL), last_conarchive_cleanup) > 5 * 60)
	{
		remove_expired_conar_entries();
		
		last_conarchive_cleanup = time(NULL);
	}
	
	return 0;
}
