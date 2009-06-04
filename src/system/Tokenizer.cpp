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


#include <cstdlib>

#include "Tokenizer.hpp"

using namespace std;

Tokenizer::Tokenizer(string sep)
{
	this->index = 0;
	this->sep = sep;
}

bool Tokenizer::isSep(char ch)
{
	for (unsigned int i=0; i < sep.length(); i++)
	{
		if (sep[i] == ch)
			return true;
	}
	
	return false;
}

bool Tokenizer::parse(const string& str)
{
	tokens.clear();
	index = 0;
	
	bool quote_open = false;
	int token_start = -1;
	
	char last_char = '\0';
	
	for (unsigned int i=0; i < str.length(); i++)
	{
		char cur_char = str[i];
		
		if (token_start != -1)
		{
			bool end_tok = false;
			int token_end = 0;
			
			if (!quote_open)
			{
				if (i == str.length()-1 && !isSep(cur_char))
				{
					end_tok = true;
					token_end = i - token_start + 1;
				}
				else if (isSep(cur_char))
				{
					end_tok = true;
					token_end = i - token_start;
				}
			}
			else if (quote_open && (cur_char == '\"' && last_char != '\\'))
			{
				end_tok = true;
				quote_open = false;
				token_end = i - token_start;
			}
			
			if (end_tok)
			{
				tokens.push_back(str.substr(token_start, token_end));
				
				token_start = -1;
			}
		}
		else
		{
			if (!isSep(cur_char))
			{
				if (cur_char == '\"' && last_char != '\\')
				{
					if (i + 1 > str.length() -1)  // sanity check
						token_start = i;
					else
						token_start = i + 1;
					
					quote_open = true;
				}
				else
				{
					if (i == str.length() -1)
						tokens.push_back(str.substr(i, 1)); // end of loop
					else
						token_start = i;
				}
			}
		}
		
		last_char = cur_char;
	}
	
	return true;
}

bool Tokenizer::getNext(string &str)
{
	if (index == count())
	{
		str = "";
		return false;
	}
	
	str = tokens[index++];
	return true;
}

string Tokenizer::getNext()
{
	if (index == count())
		return "";
	
	return tokens[index++];
}

string Tokenizer::getTillEnd(char sep)
{
	if (index == count())
		return "";
	
	string scompl;
	for (unsigned int i=index; i < count(); i++)
	{
		scompl += tokens[i];
		if (i < count() - 1)
			scompl += sep;
	}
	
	index = count();
	
	return scompl;
}

int Tokenizer::getNextInt()
{
	return string2int(getNext());
}

#if 0
float Tokenizer::getNextFloat()
{
	return string2float(getNext());
}
#endif

bool Tokenizer::popFirst()
{
	if (!count())
		return false;
	
	// move current position
	if (index)
		index--;
	
	// remove the first element
	tokens.erase(tokens.begin());
	
	return true;
}

string Tokenizer::operator[](const unsigned int i) const
{
	if (i < count())
		return tokens[i];
	else
		return "";
}

int Tokenizer::string2int(string s, unsigned int base)
{
	char *ptr;
	return strtol(s.c_str(), &ptr, base);
}

#if 0
float Tokenizer::string2float(string s)
{
	return (float)strtod(s.c_str(), NULL);
}
#endif

Tokenizer& operator>>(Tokenizer& t, int& i)
{
	i = t.getNextInt();
	return t;
}

#if 0
Tokenizer& operator>>(Tokenizer& t, float& f)
{
	f = t.getNextFloat();
	return t;
}
#endif

Tokenizer& operator>>(Tokenizer& t, string& str)
{
	str = t.getNext();
	return t;
}

Tokenizer& Tokenizer::operator--() // prefix
{
	popFirst();
	return *this;
}

