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


// Client defaults

config.set("version",		VERSION);		// config file version
config.set("default_host",	"game.holdingnuts.net");	// default host to connect to
config.set("default_port",	DEFAULT_SERVER_PORT);	// default port to connect to
config.set("auto_connect",	false);			// automatically connect to default server
config.set("player_name",	"Unnamed");		// the player name
config.set("info_location",	"");			// info: geographical location of the player
config.set("uuid",		"");			// unique ID for re-connect
config.set("locale",		"");			// language/locale to use
config.set("encoding",		"UTF-8");		// temporary fix for localized chat
config.set("log",		true);			// log to file
config.set("log_timestamp",	false);			// log with timestamp
config.set("log_chat",		false);			// include player chat in log
config.set("ui_show_handstrength", 	true);		// display hand strength on table
config.set("ui_centralized_view",	true);		// enable centralized table view
config.set("ui_card_deck",	"default");		// card deck to use
config.set("ui_echo_password",	true);			// echo password for private games by default
config.set("ui_bring_to_top",	false);			// bring window to top if player action expected
config.set("sound",		true);			// play sounds
config.set("sound_focus",	true);			// only play sound if window has focus
config.set("chat_console",	false);			// send raw commands if chat prefixed with '/'
config.set("chat_verbosity_foyer", 0x0f);		// verbosity level for foyer chat
config.set("chat_verbosity_table", 0x07);		// verbosity level for table chat

#ifdef DEBUG
config.set("dbg_register",	-1);			// automatically register to a game (value is gid; auto_connect must be set)
config.set("dbg_bbox",		false);			// show bounding boxes around scene items
config.set("dbg_srv_cmd",	false);			// log every message from server
config.set("dbg_name",		false);			// appends a random number to player name
#endif
