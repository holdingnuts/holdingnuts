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
 */


// Server defaults

config.set("port",			DEFAULT_SERVER_PORT);	// port the server is listening on
config.set("max_clients",		200);			// limit for client connections
config.set("max_connections_per_ip",	3);			// limit for connections per IP
config.set("log",			true);			// log into file
config.set("auth_password",		"");			// server authentication password
config.set("perm_create_user",		true);			// allow regular user to create games
config.set("conarchive_expire",		15 * 60);		// stored connection data expiration (seconds)
config.set("flood_chat_interval",	10);			// flood-protect: interval for measureing (seconds)
config.set("flood_chat_per_interval",	5);			// flood-protect: count of messages allowed in interval
config.set("flood_chat_mute",		60);			// flood-protect: mute time (seconds)

#ifdef DEBUG
config.set("dbg_testgame_players",	3);		// testgames with X players
config.set("dbg_testgame_games",	2);		// start X testgames
config.set("dbg_testgame_timeout",	30);		// player timeout in seconds
config.set("dbg_testgame_stakes",	1500);		// initial player stake
#endif
