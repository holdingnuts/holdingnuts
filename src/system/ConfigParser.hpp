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


typedef std::map<std::string,std::string>	cfgvars_type;


class ConfigParser
{
public:
	bool load(const char *filename);
	bool save(const char *filename);
	
	bool exists(const std::string &name) const;
	
	bool get(const std::string &name, std::string &value) const;
	std::string get(const std::string &name) const;
	
	bool getInt(const std::string &name, int &value) const;
	int getInt(const std::string &name) const;
	
	bool getBool(const std::string &name, bool &value) const;
	bool getBool(const std::string &name) const;
	
	bool set(const std::string &name, const std::string &value);
	bool set(const std::string &name, const char *value);
	bool set(const std::string &name, int value);
	bool set(const std::string &name, bool value);
	
	void print();
	
private:
	cfgvars_type vars;
};

#endif /* _CONFIGPARSER_H */
