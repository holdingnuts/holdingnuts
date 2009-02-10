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

#include <vector>
#include <map>
#include <string>

#include "Config.h"
#include "Platform.h"
#include "Network.h"
#include "Debug.h"
#include "Logger.h"
#include "Tokenizer.hpp"
#include "ConfigParser.hpp"


#include "GameController.hpp"
#include "game.hpp"

using namespace std;

extern ConfigParser config;

// temporary buffer for sending messages
static char msg[1024];

static map<int,GameController*> games;

static vector<clientcon> clients;
static unsigned int cid_counter = 0;


static clientconar_type con_archive;


GameController* get_game_by_id(int gid)
{
	map<int,GameController*>::const_iterator it = games.find(gid);
	if (it != games.end())
		return it->second;
	else
		return NULL;
}

// for pserver.cpp filling FD_SET
vector<clientcon>& get_client_vector()
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
	char buf[1024];
	const int len = snprintf(buf, sizeof(buf), "%s\r\n", message);
	const int bytes = socket_write(sock, buf, len);
	
	// FIXME: send remaining bytes if not all have been sent
	if (len != bytes)
		log_msg("clientsock", "(%d) warning: not all bytes written (%d != %d)", sock, len, bytes);
	
	return bytes;
}

bool send_response(socktype sock, bool is_success, int last_msgid, int code=0, const char *str="")
{
	char buf[128];
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
	return send_response(client->sock, true, client->last_msgid, code, str);
}

bool send_err(clientcon *client, int code=0, const char *str="")
{
	return send_response(client->sock, false, client->last_msgid, code, str);
}

// from client/foyer to client/foyer
bool client_chat(int from, int to, const char *message)
{
	char msg[1024];
	
	if (from == -1)
	{
		snprintf(msg, sizeof(msg), "MSG %d %s %s",
			from, "foyer", message);
	}
	else
	{
		clientcon* fromclient = get_client_by_id(from);
		
		snprintf(msg, sizeof(msg), "MSG %d %s %s",
			from,
			(fromclient) ? fromclient->name : "???",
			message);
	}
	
	if (to == -1)
	{
		for (vector<clientcon>::iterator e = clients.begin(); e != clients.end(); e++)
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
	char msg[1024];
	
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
	char msg[1024];
	
	clientcon* fromclient = get_client_by_id(from_cid);
	
	GameController *g = get_game_by_id(to_gid);
	if (!g)
		return false;
	
	vector<int> client_list;
	g->getPlayerList(to_tid, client_list);
	
	for (unsigned int i=0; i < client_list.size(); i++)
	{
		snprintf(msg, sizeof(msg), "MSG %d:%d:%d %s %s",
			to_gid, to_tid, from_cid,
			(fromclient) ? fromclient->name : "???",
			message);
		
		clientcon* toclient = get_client_by_id(client_list[i]);
		if (toclient)
			send_msg(toclient->sock, msg);
	}
	
	return true;
}

bool client_snapshot(int from_gid, int from_tid, int to, int sid, const char *message)
{
	snprintf(msg, sizeof(msg), "SNAP %d:%d %d %s",
		from_gid, from_tid, sid, message);
	
	clientcon* toclient = get_client_by_id(to);
	if (toclient)
		send_msg(toclient->sock, msg);
	
	return true;
}

bool client_add(socktype sock, sockaddr_in *saddr)
{
	// drop client if maximum connection count is reached
	if (clients.size() == SERVER_CLIENT_HARDLIMIT || (int)clients.size() == config.getInt("max_clients"))
	{
		send_response(sock, false, -1, ErrServerFull, "server full");
		socket_close(sock);
		
		return false;
	}
	
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
	for (vector<clientcon>::iterator client = clients.begin(); client != clients.end(); client++)
	{
		if (client->sock == sock)
		{
			socket_close(client->sock);
			
			bool send_msg = false;
			if (client->state & SentInfo)
			{
				// TODO: remove player from unstarted games
				
				snprintf(msg, sizeof(msg),
					"'%s' (%d) left foyer",
					client->name, client->id);
				
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
			
			// send client-left-msg to all remaining clients
			if (send_msg)
				client_chat(-1, -1, msg);
			
			break;
		}
	}
	
	return true;
}

int client_cmd_pclient(clientcon *client, Tokenizer &t)
{
	unsigned int version = t.getNextInt();
	string uuid = t.getNext();
	
	if (VERSION_GETMAJOR(version) != VERSION_MAJOR ||
		VERSION_GETMINOR(version) != VERSION_MINOR)
	{
		log_msg("client", "client %d version (%d) doesn't match", client->sock, version);
		send_err(client, ErrWrongVersion, "wrong version");
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
		bool use_uuid_cid = false;
		
		if (uuid.length())
		{
			clientconar_type::iterator it = con_archive.find(uuid);
			
			if (it != con_archive.end())
			{
				clientcon *conc = get_client_by_id(it->second.id);
				if (!conc)
				{
					client->id = it->second.id;
					use_uuid_cid = true;
					
					dbg_msg("uuid", "(%d) using previous cid (%d) for uuid '%s'", client->sock, client->id, client->uuid);
				}
				else
				{
					dbg_msg("uuid", "(%d) uuid '%s' already connected; used by cid %d", client->sock, client->uuid, conc->id);
					client->uuid[0] = '\0';    // client is not allowed to use this uuid
				}
			}
			else
				dbg_msg("uuid", "(%d) uuid '%s' not found", client->sock, client->uuid);
		}
		
		if (!use_uuid_cid)
			client->id = cid_counter++;
		
		
		// set temporary client name
		snprintf(client->name, sizeof(client->name), "client_%d", client->id);
		
		
		// send 'introduced response'
		snprintf(msg, sizeof(msg), "PSERVER %d %d",
			VERSION_CREATE(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION),
			client->id);
			
		send_msg(client->sock, msg);
	}
	
	return 0;
}

int client_cmd_info(clientcon *client, Tokenizer &t)
{
	string infostr;
	Tokenizer it;
	
	while (t.getNext(infostr))
	{
		it.parse(infostr, ":");
		
		string infotype, infoarg;
		it.getNext(infotype);
		
		bool havearg = it.getNext(infoarg);
		
		if (infotype == "name" && havearg)
		{
			snprintf(client->name, sizeof(client->name), "%s", infoarg.c_str());
		}
	}
	
	send_ok(client);
	
	if (!(client->state & SentInfo))
	{
		// store UUID in connection-archive
		clientcon_archive ar;
		memset(&ar, 0, sizeof(ar));
		ar.id = client->id;
		con_archive[client->uuid] = ar;
		
		// send broadcast message to foyer
		snprintf(msg, sizeof(msg),
			"'%s' (%d) joined foyer",
			client->name, client->id);
		
		client_chat(-1, -1, msg);
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
		Tokenizer ct;
		ct.parse(t.getNext(), ":");
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

int client_cmd_request(clientcon *client, Tokenizer &t)
{
	bool cmderr = false;
	
	if (!t.count())
		cmderr = true;
	else
	{
		string request = t.getNext();
		
		if (request == "clientinfo")
		{
			string scid;
			while (t.getNext(scid))   // FIXME: have maximum for count of requests
			{
				const socktype cid = Tokenizer::string2int(scid);
				const clientcon *c;
				if ((c = get_client_by_id(cid)))
				{
					snprintf(msg, sizeof(msg),
						"CLIENTINFO %d name:%s",
						cid,
						c->name);
					
					send_msg(client->sock, msg);
				}
			}
		}
		else if (request == "gameinfo")
		{
			string sgid;
			while (t.getNext(sgid))   // FIXME: have maximum for count of requests
			{
				const int gid = Tokenizer::string2int(sgid);
				const GameController *g;
				if ((g = get_game_by_id(gid)))
				{
					snprintf(msg, sizeof(msg),
						"GAMEINFO %d type:%d player:%d:%d timeout:%d",
						gid,
						(int)g->getGameType(),
						g->getPlayerMax(),
						g->getPlayerCount(),
						g->getPlayerTimeout());
					
					send_msg(client->sock, msg);
				}
			}
		}
		else if (request == "gamelist")
		{
			string gamelist;
			for (map<int,GameController*>::iterator e = games.begin(); e != games.end(); e++)
			{
				const int gid = e->first;
				
				snprintf(msg, sizeof(msg), "%d ", gid);
				gamelist += msg;
			}
			
			snprintf(msg, sizeof(msg),
				"GAMELIST %s", gamelist.c_str());
			
			send_msg(client->sock, msg);
		}
		else if (request == "playerlist")
		{
			int gid = t.getNextInt();
			
			GameController *g;
			if ((g = get_game_by_id(gid)))
			{
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
			}
		}
		else
			cmderr = true;
	}
	
	if (!cmderr)
		send_ok(client);
	else
		send_err(client);
	
	return 0;
}

int client_cmd_register(clientcon *client, Tokenizer &t)
{
	bool cmderr = false;
	
	if (!t.count())
		cmderr = true;
	else
	{
		int gid = t.getNextInt();
		GameController *g;
		if ((g = get_game_by_id(gid)))
		{
			g->removePlayer(client->id);
			
			if (g->addPlayer(client->id))
			{
				snprintf(msg, sizeof(msg),
					"'%s' (%d) joined game %d (%d/%d)",
					client->name, client->id, gid,
					g->getPlayerCount(), g->getPlayerMax());
				
				log_msg("game", "%s", msg);
				client_chat(-1, -1, msg);
			}
			else
				cmderr = true;
		}
		else
			cmderr = true;
	}
	
	if (!cmderr)
		send_ok(client);
	else
		send_err(client);
	
	return 0;
}

int client_cmd_action(clientcon *client, Tokenizer &t)
{
	bool cmderr = false;
	
	if (t.count() < 2)
		cmderr = true;
	else
	{
		int gid = t.getNextInt();
		GameController *g;
		if ((g = get_game_by_id(gid)))
		{
			string action;
			t.getNext(action);
			
			string samount;
			float amount = t.getNextFloat();
			
			Player::PlayerAction a;
			
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
				cmderr = true;
			
			if (!cmderr)
				g->setPlayerAction(client->id, a, amount);
		}
		else
			cmderr = true;
	}
	
	if (!cmderr)
		send_ok(client);
	else
		send_err(client);
	
	return 0;
}

int client_cmd_create(clientcon *client, Tokenizer &t)
{
	if (!config.getBool("perm_create_user") && !(client->state & Authed))
	{
		send_err(client, ErrNoPermission, "no permission");
		return -1;
	}
	
	//  TODO:
	
	return 0;
}

int client_cmd_auth(clientcon *client, Tokenizer &t)
{
	bool cmderr = true;
	
	if (t.count() >= 2 && config.get("auth_password").length())
	{
		int type = t.getNextInt();
		string passwd = t.getNext();
		
		// -1 is server-auth, anything other is game-auth
		if (type == -1)
		{
			if (passwd == config.get("auth_password"))
			{
				client->state |= Authed;
				
				cmderr = false;
			}
		}
		else
		{
			// TODO: game-auth
		}
	}
	
	if (!cmderr)
		send_ok(client);
	else
		send_err(client, 0, "auth failed");
	
	return 0;
}

int client_execute(clientcon *client, const char *cmd)
{
	Tokenizer t;
	t.parse(cmd);  // parse the command line
	
	// ignore blank command
	if (!t.count())
		return 0;
	
	//dbg_msg("clientsock", "(%d) executing '%s'", client->sock, cmd);
	
	// extract message-id if present
	const char firstchar = t[0][0];
	if (firstchar >= '0' && firstchar <= '9')
	{
		client->last_msgid = t.getNextInt();
		t.popFirst();
	}
	else
		client->last_msgid = -1;
	
	
	// get command argument
	const string command = t.getNext();
	t.popFirst();   // remove the command token
	
	
	if (!(client->state & Introduced))  // state: not introduced
	{
		if (command == "PCLIENT")
			client_cmd_pclient(client, t);
		else
		{
			// seems not to be a pclient
			send_err(client, ErrProtocol, "protocol error");
			client_remove(client->sock);
			return -1;
		}
	}
	else if (command == "INFO")
		client_cmd_info(client, t);
	else if (command == "CHAT")
		client_cmd_chat(client, t);
	else if (command == "REQUEST")
		client_cmd_request(client, t);
	else if (command == "REGISTER")
		client_cmd_register(client, t);
	else if (command == "ACTION")
		client_cmd_action(client, t);
	else if (command == "CREATE")
		client_cmd_create(client, t);
	else if (command == "AUTH")
		client_cmd_auth(client, t);
	else if (command == "QUIT")
	{
		send_ok(client);
		client_remove(client->sock);
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
			retval = 0;
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

int gameloop()
{
#ifdef DEBUG
	// initially add a game for debugging purpose
	if (!games.size())
	{
		for (int i=0; i < config.getInt("dbg_testgame_games"); i++)
		{
			GameController *g = new GameController();
			const int gid = i;
			g->setGameId(gid);
			g->setPlayerMax(config.getInt("dbg_testgame_players"));
			g->setPlayerTimeout(config.getInt("dbg_testgame_timeout"));
			g->setPlayerStakes(config.getInt("dbg_testgame_stakes"));
			games[gid] = g;
		}
	}
#endif
	
	// handle all games
	for (map<int,GameController*>::iterator e = games.begin(); e != games.end(); e++)
	{
		GameController *g = e->second;
		g->tick();
	}
	
	// TODO: delete all expired archived connection-data (con_archive)
	
	return 0;
}
