#ifndef _PROTOCOL_H
#define _PROTOCOL_H

typedef enum {
	SnapGameState=1,
	SnapTable,
	SnapHoleCards
} snaptype;

typedef enum {
	PlayerInRound = 0x01,
	PlayerSitout  = 0x02
} playerstate;

#endif /* _PROTOCOL_H */
