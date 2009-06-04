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
	GameModeSNG		= 0x3,  // Sit'n'Go
} gamemode;

typedef enum {
	GameStateWaiting	= 0x1,
	GameStateStarted	= 0x2,
	GameStateEnded		= 0x3,
} gamestate;

//! \brief Snapshot types
typedef enum {
	SnapGameState		= 0x01,
	SnapTable		= 0x02,
	SnapCards		= 0x03,
	SnapWinPot		= 0x07,
	SnapOddChips		= 0x08,
	SnapPlayerAction	= 0x0a,
	SnapPlayerCurrent	= 0x0b,
	SnapPlayerShow		= 0x0c,
	SnapFoyer		= 0x10,
} snaptype;

//! \brief Snapshot gamestate types
typedef enum {
	SnapGameStateStart	= 0x01,
	SnapGameStateEnd	= 0x02,
	SnapGameStateSeat	= 0x03,   // TODO
	SnapGameStateNewHand	= 0x04,
	SnapGameStateBlinds	= 0x05,
	SnapGameStateWon	= 0x10,
	SnapGameStateBroke	= 0x11,
} snap_gamestate_type;

//! \brief Snapshot cards types
typedef enum {
	SnapCardsHole		= 0x1,
	SnapCardsFlop		= 0x2,
	SnapCardsTurn		= 0x3,
	SnapCardsRiver		= 0x4,
} snap_cards_type;

//! \brief Snapshot player-action types
typedef enum {
	SnapPlayerActionFolded	= 0x01,  // cid, auto=0|1
	SnapPlayerActionChecked	= 0x02,  // cid, auto=0|1
	SnapPlayerActionCalled	= 0x03,  // cid, amount
	SnapPlayerActionBet	= 0x04,  // cid, amount
	SnapPlayerActionRaised	= 0x05,  // cid, amount
	SnapPlayerActionAllin	= 0x06,  // cid, amount
} snap_playeraction_type;

//! \brief Snapshot foyer types
typedef enum {
	SnapFoyerJoin		= 0x01,
	SnapFoyerLeave		= 0x02,
} snap_foyer_type;


typedef enum {
	PlayerInRound = 0x01,
	PlayerSitout  = 0x02,
} playerstate;

typedef enum {
	GameInfoRegistered	= 0x01,
	GameInfoPassword	= 0x02,
	GameInfoRestart		= 0x04,
	GameInfoOwner		= 0x08,
} gameinfo_flags;

typedef enum {
	ErrProtocol = 0x1,
	ErrWrongVersion = 0x2,
	ErrNotImplemented = 0x3,
	ErrParameters = 0x4,
	ErrServerFull = 0x10,
	ErrMaxConnectionsPerIP = 0x11,
	ErrNoPermission = 0x100,
} cmderror;

#endif /* _PROTOCOL_H */
