#include <iostream>
#include <string>

#include "Tokenizer.hpp"

using namespace std;

int main(void)
{
	string sa[] = {
		"Hallo",
		"Der test",
		"Test 123 \"hello bla\"",
		"bla \"1 2\" blu \"3 4\"",
		"Man sagt \"noend",
		"This isn't ended\"",
		"Quote \\\"hallo was\\\"",
		"backslash ended\\"
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
