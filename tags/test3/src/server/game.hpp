#ifndef _GAME_H
#define _GAME_H

#include "Config.h"
#include "Platform.h"
#include "Network.h"

#if 0
typedef enum {
	Collecting=1,
	Playing
} gamestate;

typedef enum {
	Settings=0x1,
	Foyer=0x2,
} snaptype;
#endif


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

#endif /* _GAME_H */
