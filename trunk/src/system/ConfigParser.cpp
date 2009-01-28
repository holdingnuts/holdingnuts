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

#include <sstream>

#include "SysAccess.h"
#include "Tokenizer.hpp"

#include "ConfigParser.hpp"

using namespace std;

bool ConfigParser::load(const char *filename)
{
	filetype *fp;
	
	fp = file_open(filename, mode_read);
	
	if (!fp)
		return false;
	
	char buffer[1024];
	
	// parse each line in config-file
	while (file_readline(fp, buffer, sizeof(buffer)))
	{
		Tokenizer t;
		t.parse(buffer);
		
		// skip blank lines
		if (!t.count())
			continue;
		
		string varname = t.getNext();
		
		// skip comments
		if (varname[0] == '#')
			continue;
		
		string value = t.getTillEnd();
		
		vars[varname] = value;
	}
	
	file_close(fp);
	
	return true;
}

bool ConfigParser::save(const char *filename)
{
	filetype *fp;
	
	fp = file_open(filename, mode_write);
	
	if (!fp)
		return false;
	
	// insert comment into file
	file_writeline(fp, "# Auto-generated HoldingNuts configuration file");
	file_writeline(fp, "");
	
	char buffer[1024];
	
	// write all config-vars
	for (map<string,string>::iterator e = vars.begin(); e != vars.end(); e++)
	{
		snprintf(buffer, sizeof(buffer), "%s  %s", e->first.c_str(), e->second.c_str());
		file_writeline(fp, buffer);
	}
	
	file_close(fp);
	
	return true;
}

bool ConfigParser::get(string name, string &value)
{
	map<string,string>::const_iterator it = vars.find(name);
	
	if (it == vars.end())
		return false;
	
	value = it->second;
	
	return true;
}

string ConfigParser::get(string name)
{
	string value = "";
	
	get(name, value);
	
	return value;
}

bool ConfigParser::getInt(string name, int &value)
{
	string svalue;
	if (!get(name, svalue))
		return false;
	
	value = Tokenizer::string2int(svalue);
	
	return true;
}

int ConfigParser::getInt(string name)
{
	int value = 0;
	
	getInt(name, value);
	
	return value;
}

bool ConfigParser::getBool(string name, bool &value)
{
	string svalue;
	if (!get(name, svalue))
		return false;
	
	if (svalue == "true" || svalue == "yes" || svalue == "1")
		value = true;
	else
		value = false;
	
	return true;
}

bool ConfigParser::getBool(string name)
{
	bool value = false;
	
	getBool(name, value);
	
	return value;
}

bool ConfigParser::set(string name, string value)
{
	vars[name] = value;
	return true;
}

bool ConfigParser::set(string name, int value)
{
	ostringstream oss;
	oss << value;
	vars[name] = oss.str();
	return true;
}

bool ConfigParser::set(string name, bool value)
{
	vars[name] = (value) ? "true" : "false";
	return true;
}
