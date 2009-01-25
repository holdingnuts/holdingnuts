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


#include "SysAccess.h"

filetype* file_open(const char *filename, int mode)
{
	char mode_str[4];
	int seq_pos = 0;
	
	if (mode & mode_read)
		mode_str[seq_pos++] = 'r';
	else if (mode & mode_write)
		mode_str[seq_pos++] = 'w';
	
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
		buf[strlen(buf)-1] = '\0';  // remove newline
	
	return s;
}

int file_writeline(filetype *fp, char *buf)
{
	int length = strlen(buf);
	file_write(fp, buf, length);
	file_write(fp, "\n", 1);
	
	return length + 1;
}

