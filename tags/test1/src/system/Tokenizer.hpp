#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <string>
#include <vector>

class Tokenizer
{
public:
	bool parse(std::string str, std::string sep = " \t\n");
	bool getNext(std::string &str);
	unsigned int getCount() const { return tokens.size(); };
	std::string operator[](const unsigned int i) const;

private:
	std::vector<std::string> tokens;
	unsigned int index;
};


// FIXME: move to somewhere else
int string2int(std::string s, unsigned int base = 0);

#endif /* _TOKENIZER_H */

