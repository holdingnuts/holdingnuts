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


#include <iostream>
#include <string>

#include "Logger.h"
#include "Debug.h"
#include "Tokenizer.hpp"
#include "ConfigParser.hpp"
#include "SysAccess.h"

using namespace std;

int test_tokenizer()
{
	string sa[] = {
		"Hallo",
		"one 1",
		"Der test",
		"Test 123 \"hello bla\"",
		"bla \"1 2\" blu \"3 4\"",
		"Man sagt \"noend",
		"This isn't ended\"",
		"Quote \\\"hallo was\\\"",
		"backslash ended\\",
		"space at end "
	};
	const int count = sizeof(sa) / sizeof(sa[0]);
	
	Tokenizer t;
	
	for (int i = 0; i < count; i++)
	{
		cout << "Test " << i + 1 << ":" << flush;
		t.parse(sa[i]);
		
		for (unsigned int j=0; j < t.count(); j++)
			cout << "_" << t[j] << "_";
		
		cout << endl << flush;
	}
	
	
	t.parse("Hallo das ist ein Test");
	
	string str;
	while (t.getNext(str))
		cout << "_" << str << "_";
	
	cout << endl << flush;
	
	
	///////////
	
	t.parse("Garbage FooBar 1 2 3");
	
	int gid, tid, cid;
	std::string name;
	
	// pop first item
	--t;
	
	// read items
	t >> name >> gid >> tid >> cid;
	
	
	cout << "gid:" << gid << " tid:" << tid << " cid:" << cid << " name:" << name << endl;
	
	return 0;
}

int test_sysaccess()
{
	filetype *fp;
	fp = file_open("testfile", mode_write | mode_read);
	
	if (!fp)
	{
		log_msg("io", "Error opening file for read/write");
		return -1;
	}
	
	char buffer[1024];
	strcpy(buffer, "Hallo\n");
	
	file_write(fp, buffer, strlen(buffer));
	
	//file_setpos(f1, 3, seek_set);
	//file_write(f1, buffer, strlen(buffer));
	
	long length = file_length(fp);
	
	log_msg("io", "length: %ld", length);
	
	file_writeline(fp, "a line");
	
	file_setpos(fp, 0, seek_set);
	
	while (file_readline(fp, buffer, sizeof(buffer)))
		log_msg("io", "line: _%s_", buffer);
	
	
	file_close(fp);
	
	return 0;
}

int test_configparser()
{
	ConfigParser cp;
	
	cp.load("settings.conf");
	
	string value = cp.get("test");
	int count = cp.getInt("count");
	
	log_msg("config", "test=_%s_  count=%d", value.c_str(), count);
	
	cp.save("settings.new.conf");
	
	return 0;
}

int main(void)
{
	//test_tokenizer();
	
	//test_sysaccess();
	
	//test_configparser();
	
	//const char *config_path = sys_config_path();
	//log_msg("sys", "config-path: _%s_", config_path);
	
	return 0;
}
