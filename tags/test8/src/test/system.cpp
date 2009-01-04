/*
 * Copyright 2008, Dominik Geyer
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
 */


#include <iostream>
#include <string>

#include "Tokenizer.hpp"

using namespace std;

int main(void)
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
		
		for (unsigned int j=0; j < t.getCount(); j++)
			cout << "_" << t[j] << "_";
		
		cout << endl << flush;
	}
	
	
	t.parse("Hallo das ist ein Test");
	
	string str;
	while (t.getNext(str))
		cout << "_" << str << "_";
	
	cout << endl << flush;
	
	return 0;
}
