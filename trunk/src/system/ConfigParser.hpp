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


#ifndef _CONFIGPARSER_H
#define _CONFIGPARSER_H

#include <string>
#include <map>

class ConfigParser
{
public:
	bool load(const char *filename);
	bool save(const char *filename);
	
	bool get(std::string name, std::string &value);
	std::string get(std::string name);
	
	bool getInt(std::string name, int &value);
	int getInt(std::string name);
	
	bool getBool(std::string name, bool &value);
	bool getBool(std::string name);
	
	bool set(std::string name, const std::string value);
	bool set(std::string name, const char *value);
	bool set(std::string name, int value);
	bool set(std::string name, bool value);
	
private:
	std::map<std::string,std::string> vars;
};

#endif /* _CONFIGPARSER_H */
