#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vector>
#include <string>

#include "Config.h"
#include "Platform.h"
#include "Network.h"
#include "Tokenizer.hpp"

#include "Debug.h"
#include "GameController.hpp"
#include "game.hpp"

using namespace std;


unsigned int snap_update = 0x0;

////////////////////////
vector<GameController*> games;

GameController* get_game_by_id(int gid)
{
	for (vector<GameController*>::iterator e = games.begin(); e != games.end(); e++)
	{
		GameController *g = *e;
		if (g->getGameId() == gid)
			return g;
	}
	return NULL;
}


////////////////////////
vector<clientcon> clients;

// for pserver.cpp filling FD_SET
void get_sock_vector(vector<socktype> &vec)
{
	vec.clear();
	
	for (vector<clientcon>::iterator e = clients.begin(); e != clients.end(); e++)
		vec.push_back(e->sock);
}

clientcon* get_client_by_sock(socktype sock)
{
	for (unsigned int i=0; i < clients.size(); i++)
		if (clients[i].sock == sock)
			return &(clients[i]);
	
	return NULL;
}

bool client_add(socktype sock)
{
	clientcon client;
	
	memset(&client, 0, sizeof(client));
	client.sock = sock;
	client.state |= Connected;
	snprintf(client.name, sizeof(client.name), "client_%d", sock);
	
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
			
			if (client->state & SentInfo)
			{
#if 0
				if (game->removePlayer((int) client->sock))
				{
					dbg_print("game", "player %d parted game (%d/%d)", client->sock,
						game->getPlayerCount(), game->getPlayerMax());
				}
#endif
				
				snap_update |= Foyer;
			}
			
			dbg_print("clientsock", "(%d) connection closed", client->sock);
			
			clients.erase(client);
			break;
		}
	}
	
	return true;
}

int send_msg(socktype sock, const char *msg)
{
	const int bufsize = 1024;
	char buf[bufsize];
	int len = snprintf(buf, bufsize, "%s\n", msg);
	int bytes = socket_write(sock, buf, len);
	
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
		clientcon* fromclient = get_client_by_sock((socktype)from);
		
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
		if (get_client_by_sock((socktype)to))
			send_msg((socktype)to, data);
		else
			return false;
	}
	
	return true;
}

// from game/game-table to client
bool client_chat(int from_gid, int from_tid, int to, const char *msg)
{
	char data[1024];
	
	snprintf(data, sizeof(data), "MSG %d:%d %s %s",
		from_gid, from_tid, "game", msg);
	
	if (get_client_by_sock((socktype)to))
		send_msg((socktype)to, data);
	
	return true;
}

int client_execute(clientcon *client, const char *cmd)
{
	socktype s = client->sock;
	char msg[1024];
	
	Tokenizer t;
	t.parse(cmd);  // parse the command line
	
	if (!t.getCount())
		return -1;
	
	dbg_print("clientsock", "(%d) executing '%s'", s, cmd);
	
	unsigned int argcount = t.getCount() - 1;
	
	// get first arg
	string command;
	t.getNext(command);
	
	bool cmderr = false;
	
	if (!(client->state & Introduced))  // state: not introduced
	{
		if (command == "PCLIENT")
		{
			unsigned int version = string2int(t[1]);
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
				send_ok(s);
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
					"client '%s' (%d) joined foyer",
					client->name, client->sock);
				
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
			string chatmsg;
			for (unsigned int i=2; i < t.getCount(); i++)
				chatmsg += t[i] + ' ';
			
			int dest = string2int(t[1]);  // FIXME: chat to table
			if (!client_chat(s, dest, chatmsg.c_str()))
				cmderr = true;
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
					socktype cid = string2int(scid);
					clientcon *client;
					if ((client = get_client_by_sock(cid)))
					{
						snprintf(msg, sizeof(msg),
							"CLIENTINFO %d name:%s",
							cid,
							client->name);
						
						send_msg(s, msg);
					}
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
			string sgid;
			t.getNext(sgid);
			
			int gid = string2int(sgid);
			GameController *g;
			if ((g = get_game_by_id(gid)))
			{
				g->removePlayer(s);
				
				if (g->addPlayer(s))
				{
					dbg_print("game", "player %d joined game (%d/%d)",
						s, g->getPlayerCount(), g->getPlayerMax());
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
#if 0
	else if (command == "UNREGISTER")
	{
		// FIXME: remove player from list when quitting
		if (game->removePlayer((int) s))
		{
			send_ok(s);
			dbg_print("game", "player %d parted game (%d/%d)", s,
				game->getPlayerCount(), game->getPlayerMax());
			
			client->state &= ~IsPlayer;
			snap_update |= Foyer;
		}
		else
			send_err(s);
	}
#endif
	else if (command == "AUTH")
	{
		if (argcount < 2)
			cmderr = true;
		else
		{
			int type = string2int(t[1]);  // FIXME: parse gid:tid
			
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
			client->msgbuf[i] = ' ';  // debugging purpose
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

#if 0
int send_snapshot(snaptype type)
{
	char msg[1024];
	
	if (type == Foyer)
	{
		snprintf(msg, sizeof(msg), "SNAP foyer %s", foyer_snapshot.c_str());
	}
	else if (type == Table)
	{
		strcpy(msg, "SNAP table ");
	}
	
	// send all clients current game-state
	for (unsigned int i=0; i < clients.size(); i++)
		send_msg(clients[i].sock, msg);
	
	return 0;
}

int update_foyer_snapshot()
{
	foyer_snapshot.clear();
	
	for (unsigned int i=0; i < clients.size(); i++)
	{
		//if (!(clients[i].state & SentInfo))
		//	continue;
		
		char tmp[100];
		snprintf(tmp, sizeof(tmp), "%d:%c:%s ",
			clients[i].sock,
			(clients[i].state & IsPlayer) ? '*' : '-',
			clients[i].name);
		
		foyer_snapshot += tmp;
	}
	
	dbg_print("snapshot", "foyer size: %d", (int)foyer_snapshot.length());
	
	return 0;
}
#endif

int gameloop()
{
#if 0
	// create and initialize a gamecontroller
	if (!game)
	{
		game = new GameController();
		game->setPlayerMax(2);
	}
	
	if (snap_update)
	{
		if (snap_update & Foyer)
		{
			update_foyer_snapshot();
			send_snapshot(Foyer);
		}
		
		snap_update = 0x0;
	}
#endif
	
	// initially add a game for debugging purpose
	if (!games.size())
	{
		GameController *g = new GameController();
		g->setGameId(0);
		g->setPlayerMax(2);
		games.push_back(g);
	}
	
	for (vector<GameController*>::iterator e = games.begin(); e != games.end(); e++)
	{
		GameController *g = *e;
		g->tick();
	}
	
	return 0;
}

