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

typedef enum {
	ErrProtocol = 0x1,
	ErrWrongVersion = 0x2,
	ErrNotImplemented = 0x3,
	ErrServerFull = 0x10,
	ErrNoPermission = 0x100
} cmderror;

#endif /* _PROTOCOL_H */
