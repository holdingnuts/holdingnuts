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


#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <string>
#include <vector>

class Tokenizer
{
public:
	bool parse(std::string str, std::string sep = " \t\n");
	bool getNext(std::string &str);
	std::string getNext();
	std::string getTillEnd(char sep=' ');
	int getNextInt();
	float getNextFloat();
	
	unsigned int getCount() const { return tokens.size(); };
	std::string operator[](const unsigned int i) const;
	
	static bool isSep(char ch, std::string sep);
	static int string2int(std::string s, unsigned int base = 0);
	static float string2float(std::string s);

private:
	std::vector<std::string> tokens;
	unsigned int index;
};

#endif /* _TOKENIZER_H */
