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
# include <windows.h>
# include <dirent.h>
# include <io.h>
# include <tchar.h>
#else
# include <unistd.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include "SysAccess.h"
#include "Debug.h"


// FIXME: have platform independent solution instead of fix value
#define MAX_PATH_LEN	4096

static char cfg_path[MAX_PATH_LEN + 1] = "";


const char* build_mode_string(int mode)
{
	static char mode_str[4];
	char *ms = mode_str;
	
	if (mode & mode_read && mode & mode_write)
	{
		*ms++ = 'w';
		*ms++ = '+';
	}
	else if (mode & mode_read && mode & mode_append)
	{
		*ms++ = 'a';
		*ms++ = '+';
	}
	else
	{
		if (mode & mode_read)
			*ms++ = 'r';
		
		if (mode & mode_write)
			*ms++ = 'w';
		
		if (mode & mode_append)
			*ms++ = 'a';
	}
	
#if defined(PLATFORM_WINDOWS)
	*ms++ = 'b';
#endif
	
	*ms = '\0';
	
	return mode_str;
}

filetype* file_open(const char *filename, int mode)
{
	return fopen(filename, build_mode_string(mode));
}

filetype* file_reopen(const char *filename, int mode, filetype *stream)
{
	return freopen(filename, build_mode_string(mode), stream);
}

int file_close(filetype *fp)
{
	return fclose(fp);
}

size_t file_read(filetype *fp, void *buf, unsigned int size)
{
	return fread(buf, 1, size, fp);
}

size_t file_write(filetype *fp, const void *buf, unsigned int size)
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

int file_writeline(filetype *fp, const char *buf)
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

int sys_isdir(const char *path)
{
	struct stat sb;
	if (stat(path, &sb) == -1)
		return 0;

	if (S_ISDIR(sb.st_mode))
		return 1;
	
	return 0;
}

int sys_chdir(const char *path)
{
	if (chdir(path) == -1)
		return -1;
	
	return 0;
}

int sys_set_config_path(const char *path)
{
	snprintf(cfg_path, sizeof(cfg_path), "%s", path);
	return 0;
}

const char* sys_config_path()
{
	static char path[MAX_PATH_LEN + 1];
	const char *config_path;
	
	// use manually set config-directory
	if (*cfg_path)
		return cfg_path;
	
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

const char* sys_data_path()
{
	unsigned int i;
	static char *search_dirs[] = {
#if defined(DATA_DIR)
		DATA_DIR,
#else
		"data",
#if !defined(PLATFORM_WINDOWS)
		"/usr/share/" CONFIG_APPNAME "/data",
		"/usr/share/games/" CONFIG_APPNAME "/data",
		"/usr/local/share/" CONFIG_APPNAME "/data",
		"/usr/local/games/" CONFIG_APPNAME "/data",
		"/usr/local/share/games/" CONFIG_APPNAME "/data",
		"/opt/" CONFIG_APPNAME "/data",
#endif
#endif /* defined(DATA_DIR) */
	};
	
	const unsigned int count = sizeof(search_dirs) / sizeof(search_dirs[0]);
	
	for (i=0; i < count; i++)
	{
		// FIXME: ~/data could be found if run from $HOME.
		//        add an additional check for present file/dir in data-dir
		if (sys_isdir(search_dirs[i]))
			return search_dirs[i];
	}
	
	return 0;
}

const char* sys_username()
{
	static char username[128];
	
	*username = '\0';
	
#if defined(PLATFORM_WINDOWS)
	TCHAR userBuf[128];
	DWORD bufCount = sizeof(userBuf);
	if (GetUserName(userBuf, &bufCount))
		snprintf(username, sizeof(username), "%s", (char*) userBuf);  // FIXME: assuming (TCHAR == char)
#else
	snprintf(username, sizeof(username), "%s", getenv("USER"));
#endif
	return username;
}
