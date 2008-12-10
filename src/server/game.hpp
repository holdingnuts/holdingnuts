#ifndef _GAME_H
#define _GAME_H

#include "Config.h"
#include "Platform.h"
#include "Network.h"


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


typedef enum {
	SnapGameState=1,
	SnapTable,
	SnapPlayerStats
} snaptype;



clientcon* get_client_by_sock(socktype sock);
bool client_chat(int from_gid, int from_tid, int to, const char *msg);
bool client_snapshot(int from_gid, int from_tid, int to, int sid, const char *msg);


#endif /* _GAME_H */
