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
#include <time.h>
#include <string.h>

#include "Logger.h"


#if defined __cplusplus
        extern "C" {
#endif

static filetype *logger[2] = { 0, 0 };
static int log_timestamp = 0;

void log_msg(const char *level, const char *format, ...)
{
	va_list args;
	char msg[1024];
	unsigned int i;
	
	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);
	
	// if there was no log target specified with log_set()
	if (!logger[0])
		logger[0] = stderr;
	
	for (i=0; i < 2; i++)
	{
		if (!logger[i])
			continue;
		
		if (log_timestamp)
			fprintf(logger[i], "[%ld %10s]  %s\n", time(0), level, msg);
		else
			fprintf(logger[i], "[%10s]  %s\n", level, msg);
		
		fflush(logger[i]);
	}
}

void log_set(filetype *stream1, filetype *stream2)
{
	logger[0] = stream1;
	logger[1] = stream2;
}

void log_use_timestamp(int use_timestamp)
{
	log_timestamp = use_timestamp;
}

#if defined __cplusplus
    }
#endif
