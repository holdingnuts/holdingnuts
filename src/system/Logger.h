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


#ifndef _LOGGER_H
#define _LOGGER_H


#include "Platform.h"
#include "SysAccess.h"


#if defined __cplusplus
        extern "C" {
#endif


void log_msg(const char *level, const char *format, ...);
void log_set(filetype *stream1, filetype *stream2);
void log_use_timestamp(int use_timestamp);

#if defined __cplusplus
    }
#endif

#endif /* _LOGGER_H */
