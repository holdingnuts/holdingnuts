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
#include "Tokenizer.hpp"
#include "ConfigParser.hpp"

#include "Debug.h"
#include "GameController.hpp"
#include "game.hpp"

using namespace std;

extern ConfigParser config;

////////////////////////
map<int,GameController*> games;

GameController* get_game_by_id(int gid)
{
	map<int,GameController*>::const_iterator it = games.find(gid);
	if (it != games.end())
		return it->second;
	else
		return NULL;
}


////////////////////////
vector<clientcon> clients;
unsigned int cid_counter = 0;

map<string,clientcon_archive> con_archive;

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

int send_msg(socktype sock, const char *msg)
{
	char buf[1024];
	const int len = snprintf(buf, sizeof(buf), "%s\r\n", msg);
	const int bytes = socket_write(sock, buf, len);
	
	// FIXME: send remaining bytes if not all have been sent
	if (len != bytes)
		dbg_print("clientsock", "(%d) warning: not all bytes written (%d != %d)", sock, len, bytes);
	
	return bytes;
}

bool send_ok(socktype sock, int code=0, const char *str="")
{
	char msg[128];
	snprintf(msg, sizeof(msg), "OK %d %s", code, str);
	return send_msg(sock, msg);
}

bool send_err(socktype sock, int code=0, const char *str="")
{
	char msg[128];
	snprintf(msg, sizeof(msg), "ERR %d %s", code, str);
	return send_msg(sock, msg);
}

// from client/foyer to client/foyer
bool client_chat(int from, int to, const char *msg)
{
	char data[1024];
	
	if (from == -1)
	{
		snprintf(data, sizeof(data), "MSG %d %s %s",
			from, "foyer", msg);
	}
	else
	{
		clientcon* fromclient = get_client_by_id(from);
		
		snprintf(data, sizeof(data), "MSG %d %s %s",
			from,
			(fromclient) ? fromclient->name : "???",
			msg);
	}
	
	if (to == -1)
	{
		for (vector<clientcon>::iterator e = clients.begin(); e != clients.end(); e++)
			send_msg(e->sock, data);
	}
	else
	{
		clientcon* toclient = get_client_by_id(to);
		if (toclient)
			send_msg(toclient->sock, data);
		else
			return false;
	}
	
	return true;
}

// from game/table to client
bool client_chat(int from_gid, int from_tid, int to, const char *msg)
{
	char data[1024];
	
	snprintf(data, sizeof(data), "MSG %d:%d %s %s",
		from_gid, from_tid, (from_tid == -1) ? "game" : "table", msg);
	
	clientcon* toclient = get_client_by_id(to);
	if (toclient)
		send_msg(toclient->sock, data);
	
	return true;
}

// from client to game/table
bool table_chat(int from_cid, int to_gid, int to_tid, const char *msg)
{
	char data[1024];
	
	clientcon* fromclient = get_client_by_id(from_cid);
	
	GameController *g = get_game_by_id(to_gid);
	if (!g)
		return false;
	
	vector<int> client_list;
	g->getPlayerList(to_tid, client_list);
	
	for (unsigned int i=0; i < client_list.size(); i++)
	{
		snprintf(data, sizeof(data), "MSG %d:%d:%d %s %s",
			to_gid, to_tid, from_cid,
			(fromclient) ? fromclient->name : "???",
			msg);
		
		clientcon* toclient = get_client_by_id(client_list[i]);
		if (toclient)
			send_msg(toclient->sock, data);
	}
	
	return true;
}

bool client_snapshot(int from_gid, int from_tid, int to, int sid, const char *msg)
{
	char data[1024];
	
	snprintf(data, sizeof(data), "SNAP %d:%d %d %s",
		from_gid, from_tid, sid, msg);
	
	clientcon* toclient = get_client_by_id(to);
	if (toclient)
		send_msg(toclient->sock, data);
	
	return true;
}

bool client_add(socktype sock, sockaddr_in *saddr)
{
	// drop client if maximum connection count is reached
	if (clients.size() == SERVER_CLIENT_HARDLIMIT || (int)clients.size() == config.getInt("max_clients"))
	{
		send_err(sock, -999 /* FIXME: error-code */, "server full");
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
	char msg[1024];
	
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
					con_archive[uuid].id = client->id;
					con_archive[uuid].logout_time = time(NULL);
					//con_archive[uuid].saddr = client->saddr;
				}
			}
			
			dbg_print("clientsock", "(%d) connection closed", client->sock);
			
			clients.erase(client);
			
			// send client-left-msg to all remaining clients
			if (send_msg)
				client_chat(-1, -1, msg);
			
			break;
		}
	}
	
	return true;
}

int client_execute(clientcon *client, const char *cmd)
{
	socktype s = client->sock;
	char msg[1024];
	
	Tokenizer t;
	t.parse(cmd);  // parse the command line
	
	if (!t.getCount())
		return 0;
	
	//dbg_print("clientsock", "(%d) executing '%s'", s, cmd);
	
	unsigned int argcount = t.getCount() - 1;
	
	// get first arg
	string command;
	t.getNext(command);
	
	bool cmderr = false;
	
	if (!(client->state & Introduced))  // state: not introduced
	{
		if (command == "PCLIENT")
		{
			unsigned int version = t.getNextInt();
			string uuid = t.getNext();
			
			if (VERSION_GETMAJOR(version) != VERSION_MAJOR ||
				VERSION_GETMINOR(version) != VERSION_MINOR)
			{
				dbg_print("client", "client %d version (%d) doesn't match", s, version);
				send_err(s);
			}
			else
			{
				client->version = version;
				client->state |= Introduced;
				
				snprintf(client->uuid, sizeof(client->uuid), "%s", uuid.c_str());
				
				// re-assign cid if this client was previously connected (and cid isn't already connected)
				if (uuid.length() && con_archive.find(uuid) != con_archive.end() && !get_client_by_id(con_archive[uuid].id))
					client->id = con_archive[uuid].id;
				else
					client->id = cid_counter++;
				
				// set temporary client name
				snprintf(client->name, sizeof(client->name), "client_%d", client->id);
				
				
				// send 'introduced response'
				snprintf(msg, sizeof(msg), "PSERVER %d %d",
					VERSION_CREATE(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION),
					client->id);
					
				send_msg(client->sock, msg);
			}
		}
		else
		{
			send_err(s, 1, "protocol error");
			client_remove(s);
			return -1;
		}
	}
	else if (command == "INFO")
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
		
		if (!cmderr)
		{
			send_ok(s);
			
			if (!(client->state & SentInfo))
			{
				snprintf(msg, sizeof(msg),
					"'%s' (%d) joined foyer",
					client->name, client->id);
				
				client_chat(-1, -1, msg);
			}
			
			client->state |= SentInfo;
		}
		else
			send_err(s);
	}
	else if (command == "CHAT")
	{
		if (argcount < 2)
			cmderr = true;
		else
		{
			Tokenizer ct;
			ct.parse(t.getNext(), ":");
			string chatmsg = t.getTillEnd();
			
			if (ct.getCount() == 1) // cid
			{
				int dest = ct.getNextInt();
				
				if (!client_chat(client->id, dest, chatmsg.c_str()))
					cmderr = true;
			}
			else if (ct.getCount() == 2)  // gid:tid
			{
				int gid = ct.getNextInt();
				int tid = ct.getNextInt();
				
				if (!table_chat(client->id, gid, tid, chatmsg.c_str()))
					cmderr = true;
			}
		}
		
		if (!cmderr)
			send_ok(s);
		else
			send_err(s);
	}
	else if (command == "REQUEST")
	{
		if (!argcount)
			cmderr = true;
		else
		{
			string request;
			t.getNext(request);
			
			if (request == "clientinfo")
			{
				string scid;
				while (t.getNext(scid))   // FIXME: have maximum for count of requests
				{
					socktype cid = Tokenizer::string2int(scid);
					clientcon *client;
					if ((client = get_client_by_id(cid)))
					{
						snprintf(msg, sizeof(msg),
							"CLIENTINFO %d name:%s",
							cid,
							client->name);
						
						send_msg(s, msg);
					}
				}
			}
			else if (request == "gamelist")
			{
				string gameinfo;
				for (map<int,GameController*>::iterator e = games.begin(); e != games.end(); e++)
				{
					int gid = e->first;
					GameController *g = e->second;
					char tmp[128];
					
					snprintf(tmp, sizeof(tmp), "%d:%d:moreinfo ",
						gid, (int)g->getGameType());
					
					gameinfo += tmp;
				}
				
				snprintf(msg, sizeof(msg),
					"GAMELIST %s", gameinfo.c_str());
				
				send_msg(s, msg);
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
						char tmp[10];
						snprintf(tmp, sizeof(tmp), "%d ", client_list[i]);
						slist += tmp;
					}
					
					snprintf(msg, sizeof(msg), "PLAYERLIST %d %s", gid, slist.c_str());
					send_msg(s, msg);
				}
			}
			else
				cmderr = true;
		}
		
		if (!cmderr)
			send_ok(s);
		else
			send_err(s);
	}
	else if (command == "REGISTER")
	{
		if (!argcount)
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
					
					dbg_print("game", "%s", msg);
					client_chat(-1, -1, msg);
				}
				else
					cmderr = true;
			}
			else
				cmderr = true;
		}
		
		if (!cmderr)
			send_ok(s);
		else
			send_err(s);
	}
	else if (command == "ACTION")
	{
		if (argcount < 2)
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
			send_ok(s, 0);
		else
			send_err(s, 0, "what?");
	}
	else if (command == "AUTH")
	{
		if (argcount < 2)
			cmderr = true;
		else
		{
			int type = t.getNextInt();  // FIXME: -1=server | other=game_id
			
			snprintf(msg, sizeof(msg), "auth on %d", type);
			
			if (t[2] == "secret")  // FIXME: no default pw, only testing here
				client->state |= Authed;
			else
				cmderr = true;
		}
		
		if (!cmderr)
			send_ok(s, 0, msg);
		else
			send_err(s, 0, "auth failed");
	}
	else if (command == "QUIT")
	{
		send_ok(s);
		client_remove(s);
		return -1;
	}
	else
	{
		send_err(s, 10, "not implemented");
	}
	
	return 0;
}

// returns zero if no cmd was found or no bytes remaining after exec
int client_parsebuffer(clientcon *client)
{
	//dbg_print("clientsock", "(%d) parse (bufferlen=%d)", client->sock, client->buflen);
	
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
		
		//dbg_print("clientsock", "(%d) command: '%s' (len=%d)", client->sock, cmd, found_nl);
		if (client_execute(client, cmd) != -1)  // client quitted ?
		{
			// move the rest to front
			memmove(client->msgbuf, client->msgbuf + found_nl + 1, client->buflen - (found_nl + 1));
			client->buflen -= found_nl + 1;
			//dbg_print("clientsock", "(%d) new buffer after cmd (bufferlen=%d)", client->sock, client->buflen);
			
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
	
	
	//dbg_print("clientsock", "(%d) DATA len=%d", sock, bytes);
	
	clientcon *client = get_client_by_sock(sock);
	if (!client)
	{
		dbg_print("clientsock", "(%d) error: no client associated", sock);
		return -1;
	}
	
	if (client->buflen + bytes > (int)sizeof(client->msgbuf))
	{
		dbg_print("clientsock", "(%d) error: buffer size exceeded", sock);
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
	// initially add a game for debugging purpose
	if (!games.size())
	{
		for (int i=0; i < config.getInt("dbg_testgame_games"); i++)
		{
			GameController *g = new GameController();
			const int gid = i;
			g->setGameId(gid);
			g->setPlayerMax(config.getInt("dbg_testgame_players"));
			games[gid] = g;
		}
	}
	
	// handle all games
	for (map<int,GameController*>::iterator e = games.begin(); e != games.end(); e++)
	{
		GameController *g = e->second;
		g->tick();
	}
	
	// TODO: delete all expired archived connection-data (con_archive)
	
	return 0;
}

