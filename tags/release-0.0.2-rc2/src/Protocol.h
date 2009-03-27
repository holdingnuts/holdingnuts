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


#ifndef _PROTOCOL_H
#define _PROTOCOL_H

typedef enum {
	GameTypeHoldem = 0x1
} gametype;

typedef enum {
	GameModeRingGame	= 0x1, // Cash game
	GameModeFreezeOut	= 0x2, // Tournament
	GameModeSNG		= 0x3  // Sit'n'Go
} gamemode;

typedef enum {
	GameStateWaiting	= 0x1,
	GameStateStarted	= 0x2,
	GameStateEnded		= 0x3
} gamestate;

typedef enum {
	SnapGameState = 0x1,
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
	ErrParameters = 0x4,
	ErrServerFull = 0x10,
	ErrMaxConnectionsPerIP = 0x11,
	ErrNoPermission = 0x100
} cmderror;

#endif /* _PROTOCOL_H */
