/*
 * Copyright 2008, Dominik Geyer
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


#ifndef _CONFIG_H
#define _CONFIG_H

#include "Platform.h"

/* version(major,minor,revision) */
#define VERSION_MAJOR     0
#define VERSION_MINOR     0
#define VERSION_REVISION  1

#define DEFAULT_SERVER_PORT  12345
#define MAX_PLAYERS  10

#define SERVER_LISTEN_BACKLOG       3
#define SERVER_SELECT_TIMEOUT_SEC   0
#define SERVER_SELECT_TIMEOUT_USEC  150000  /* 150ms */

/* server testing; verbose messages */
//#define SERVER_TESTING


#define CLIENT_CONNECT_TIMEOUT     10


/* TODO: move to other file
   Example: 2.12.123 = 2*100000 + 12*1000 + 123 = 212123
 */
#define VERSION_CREATE(major,minor,revision) \
	(major*100000 + minor*1000 + revision)

#define VERSION_GETMAJOR(version) \
	(version / 100000)

#define VERSION_GETMINOR(version) \
	((version - VERSION_GETMAJOR(version)*100000) / 1000)

#define VERSION_GETREVISION(version) \
	((version - VERSION_GETMAJOR(version)*100000) - VERSION_GETMINOR(version)*1000)

#endif /* _CONFIG_H */
