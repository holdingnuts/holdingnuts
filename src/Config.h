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


#ifndef _CONFIG_H
#define _CONFIG_H

#include "Version.h"
#include "Platform.h"


/* the default port the server is listening on */
#define DEFAULT_SERVER_PORT  40888

/* the hard-limit of clients which can connect */
#define SERVER_CLIENT_HARDLIMIT  1000

/* max pending connections for listening socket */
#define SERVER_LISTEN_BACKLOG  3

/* time to wait for an action on the non-blocking sockets (in millisecs) */
#define SERVER_SELECT_TIMEOUT_USEC  150000

/* server testing mode used in test-programs (define to enable) */
#undef SERVER_TESTING


/* client manual website; menu Help->Handbook */
#define CLIENT_WEBSITE_HELP  "http://wiki.holdingnuts.net/handbook"


#if defined(PLATFORM_WINDOWS)
# define CONFIG_APPNAME "HoldingNuts"
#else
# define CONFIG_APPNAME "holdingnuts"
#endif

/* hard-coded path to data-dir */
//#define DATA_DIR  "/usr/local/share"

#endif /* _CONFIG_H */
