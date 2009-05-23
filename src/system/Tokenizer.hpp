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
	Tokenizer(std::string sep = " \t\n");
	
	bool parse(const std::string& str);
	bool getNext(std::string &str);
	std::string getNext();
	std::string getTillEnd(char sep=' ');
	int getNextInt();
	//float getNextFloat();
	
	bool popFirst();
	
	unsigned int count() const { return tokens.size(); };
	std::string operator[](const unsigned int i) const;
	
	bool isSep(char ch);
	
	static int string2int(std::string s, unsigned int base = 0);
	//static float string2float(std::string s);
	
	friend Tokenizer& operator>>(Tokenizer& left, int& i);
	//friend Tokenizer& operator>>(Tokenizer& left, float& f);
	friend Tokenizer& operator>>(Tokenizer& left, std::string& str);
	Tokenizer& operator--(); // prefix


private:
	std::vector<std::string> tokens;
	unsigned int index;
	
	std::string sep;
};

#endif /* _TOKENIZER_H */
