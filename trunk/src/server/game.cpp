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

using namespace std;


GameController *game = NULL;

typedef enum {
	Collecting=1,
	Playing
} gamestate;

typedef enum {
	Settings=0x1,
	Foyer=0x2,
	Table=0x4
} snaptype;


unsigned int snap_update = 0x0;

string foyer_snapshot;

typedef enum {
	Connected = 0x01,
	Introduced = 0x02,
	SentInfo = 0x04,
	Authed = 0x08,
	IsPlayer = 0x10
} clientstate;

typedef struct {
	socktype sock;
	//time_t lastdata;
	
	char msgbuf[1024];
	int buflen;
	
	unsigned int state;
	unsigned int version;
	char name[64];
} clientcon;


vector<clientcon> clients;

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
				if (game->removePlayer((int) client->sock))
				{
					dbg_print("game", "player %d parted game (%d/%d)", client->sock,
						game->getPlayerCount(), game->getPlayerMax());
				}
				
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

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
{
	// Skip delimiters at beginning.
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos     = str.find_first_of(delimiters, lastPos);
	
	while (string::npos != pos || string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

bool client_chat(int from, int to, char *msg)
{
	char data[1024];
	clientcon* fromclient = get_client_by_sock((socktype)from);
	
	snprintf(data, sizeof(data), "MSG %d %s %s",
		from,
		(fromclient) ? fromclient->name : "???",
		msg);
	
	if (to == -1)
	{
		for (vector<clientcon>::iterator e = clients.begin(); e != clients.end(); e++)
		{
			//if (e->sock != (socktype)from)
				send_msg(e->sock, data);
		}
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


int client_execute(clientcon *client, const char *cmd)
{
	socktype s = client->sock;
	char msg[1024];
	
	string scmd = cmd;
	vector<string> tokens;
	
	Tokenize(scmd, tokens, " ");
	if (!tokens.size())
		return -1;
	
	dbg_print("clientsock", "(%d) trying to execute '%s'", s, cmd);
	
	
	string command = tokens[0];
	
	if (!(client->state & Introduced))
	{
		if (command == "PCLIENT")
		{
			unsigned int version = atol(tokens[1].c_str());
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
		string infotype = tokens[1];
		
		if (infotype == "name")
		{
			if (client->state & SentInfo)
				send_err(s, 0, "name already set");
			else
			{
				// FIXME: check for invalid chars and null-string
				snprintf(client->name, sizeof(client->name), "%s", tokens[2].c_str());
				
				dbg_print("foyer", "player '%s' (%d) joined foyer", client->name, s);
				
				client->state |= SentInfo;
				
				snap_update |= Foyer;
				
				send_ok(s);
			}
		}
	}
	//else if (!(client->state & SentInfo))
	//{
	//	send_msg(s, "ERR 2 info needed first");
	//}
	else if (command == "CHAT")
	{
		if (tokens.size() < 3)
			return -1;
		
		msg[0] = '\0';
		for (unsigned int i=2; i < tokens.size(); i++)
		{
			// FIXME: buffer overflow AND DIRTY DIRTY
			strcat(msg, tokens[i].c_str());
			strcat(msg, " ");
		}
		
		int dest = atol(tokens[1].c_str());
		if (client_chat(s, dest, msg))
			send_ok(s);
		else
			send_err(s);
	}
	else if (command == "JOIN")
	{
		game->removePlayer((int) s);  // remove if already present
		
		if (game->addPlayer((int) s, client->name))
		{
			send_ok(s);
			dbg_print("game", "player %d joined game (%d/%d)", s,
				game->getPlayerCount(), game->getPlayerMax());
			
			client->state |= IsPlayer;
			snap_update |= Foyer;
		}
		else
			send_err(s);
	}
	else if (command == "PART")
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
	else if (command == "AUTH")
	{
		if (tokens[1] == "secret")
		{
			client->state |= Authed;
			send_ok(s);
		}
		else
			send_err(s, 0, "auth faild");
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

int gameloop()
{
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
	
	return 0;
}

