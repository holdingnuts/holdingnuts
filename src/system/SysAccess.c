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

#include "Config.h"

#include <errno.h>

#if defined(PLATFORM_WINDOWS)
#else
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include "SysAccess.h"

filetype* file_open(const char *filename, int mode)
{
	char mode_str[4];
	int seq_pos = 0;
	
	if (mode & mode_read && mode & mode_write)
	{
		mode_str[seq_pos++] = 'w';
		mode_str[seq_pos++] = '+';
	}
	else if (mode & mode_read && mode & mode_append)
	{
		mode_str[seq_pos++] = 'a';
		mode_str[seq_pos++] = '+';
	}
	else
	{
		if (mode & mode_read)
			mode_str[seq_pos++] = 'r';
		
		if (mode & mode_write)
			mode_str[seq_pos++] = 'w';
		
		if (mode & mode_append)
			mode_str[seq_pos++] = 'a';
	}
	
#if defined(PLATFORM_WINDOWS)
	mode_str[seq_pos++] = 'b';
#endif
	
	mode_str[seq_pos++] = '\0';
	
	return fopen(filename, mode_str);
}

int file_close(filetype *fp)
{
	return fclose(fp);
}

size_t file_read(filetype *fp, void *buf, unsigned int size)
{
	return fread(buf, 1, size, fp);
}

size_t file_write(filetype *fp, void *buf, unsigned int size)
{
	return fwrite(buf, 1, size, fp);
}

int file_setpos(filetype *fp, long offset, int whence)
{
	int pos = SEEK_SET;
	
	switch (whence)
	{
	case seek_set:
		pos = SEEK_SET;
		break;
	case seek_cur:
		pos = SEEK_CUR;
		break;
	case seek_end:
		pos = SEEK_END;
		break;
	}
	
	return fseek(fp, offset, pos);
}

long file_getpos(filetype *fp)
{
	return ftell(fp);
}

long file_length(filetype *fp)
{
	long cur, length;
	
	cur = file_getpos(fp);
	file_setpos(fp, 0, seek_end);
	length = file_getpos(fp);
	file_setpos(fp, cur, seek_set);
	
	return length;
}

char* file_readline(filetype *fp, char *buf, int max)
{
	char *s;
	s = fgets(buf, max, fp);
	
	if (s)
	{
		// remove newline
		int length = strlen(buf);
		
		if (length >= 1 && (buf[length - 1] == '\n' || buf[length - 1] == '\r'))
			buf[length - 1] = '\0';
		if (length >= 2 && (buf[length - 2] == '\n' || buf[length - 2] == '\r'))
			buf[length - 2] = '\0';
		
	}
	
	return s;
}

int file_writeline(filetype *fp, char *buf)
{
	int length = strlen(buf);
	file_write(fp, buf, length);
	file_write(fp, "\n", 1);
	
	return length + 1;
}

int sys_mkdir(const char *path)
{
#if defined(PLATFORM_WINDOWS)
	if (_mkdir(path) == 0)
		return 0;
#else
	if (mkdir(path, 0755) == 0)
		return 0;
#endif
	
	// don't indicate error if directory already exists
	if(errno == EEXIST)
		return 0;
	
	return -1;
}

const char* sys_config_path()
{
	static char path[1024];
	const char *config_path;

#if defined(PLATFORM_WINDOWS)
	config_path = getenv("APPDATA");
#else
	config_path = getenv("HOME");
#endif
	
	if (!config_path)
		return 0;
	
#if defined(PLATFORM_WINDOWS)
	snprintf(path, sizeof(path), "%s/%s", config_path, CONFIG_APPNAME);
#else
	snprintf(path, sizeof(path), "%s/.%s", config_path, CONFIG_APPNAME);
#endif
	
	return path;
}
