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


#ifndef _SYSACCESS_H
#define _SYSACCESS_H

#include "Platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if defined __cplusplus
        extern "C" {
#endif


typedef FILE filetype;

typedef enum {
	mode_read	= 0x01,
	mode_write	= 0x02,
	mode_append	= 0x04
} filemode;

typedef enum {
	seek_set	= 0x01,
	seek_cur	= 0x02,
	seek_end	= 0x04
} seekpos;


filetype* file_open(const char *filename, int mode);
filetype* file_reopen(const char *filename, int mode, filetype *stream);
int file_close(filetype *fp);
size_t file_read(filetype *fp, void *buf, unsigned int size);
size_t file_write(filetype *fp, const void *buf, unsigned int size);
int file_setpos(filetype *fp, long offset, int whence);
long file_getpos(filetype *fp);
long file_length(filetype *fp);

char* file_readline(filetype *fp, char *buf, int max);
int file_writeline(filetype *fp, const char *buf);

int sys_mkdir(const char *path);
int sys_isdir(const char *path);
int sys_chdir(const char *path);

int sys_set_config_path(const char *path);
const char* sys_config_path();

const char* sys_data_path();
const char* sys_username();

#if defined __cplusplus
    }
#endif

#endif /* _SYSACCESS_H */
