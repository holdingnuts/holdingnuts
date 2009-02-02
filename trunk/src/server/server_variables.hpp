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

config.set("port",		DEFAULT_SERVER_PORT);	// port the server is listening on
config.set("max_clients",	200);			// limit for client connections
config.set("log",		true);			// log into file
config.set("auth_password",	"");			// server authentication password
config.set("perm_create_user",	true);			// allow regular user to create games


#ifdef DEBUG
config.set("dbg_testgame_players",	3);
config.set("dbg_testgame_games",	2);
#endif
