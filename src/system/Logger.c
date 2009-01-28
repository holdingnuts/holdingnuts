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


#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "Logger.h"


#if defined __cplusplus
        extern "C" {
#endif

static filetype *logger[2] = { 0, 0 };


void dbg_print(const char *level, const char *format, ...)
{
	va_list args;
	char msg[1024];
	unsigned int i;
	
	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);
	
	// if there was no log target specified with dbg_setlog()
	if (!logger[0])
		logger[0] = stderr;
	
	for (i=0; i < 2; i++)
	{
		if (!logger[i])
			continue;
		
		fprintf(logger[i], "[%s]: %s\n", level, msg);
		fflush(logger[i]);
	}
}

void dbg_setlog(filetype *stream1, filetype *stream2)
{
	logger[0] = stream1;
	logger[1] = stream2;
}

#if defined __cplusplus
    }
#endif
