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
	std::string operator[](const unsigned int i) const { return tokens[i]; };

private:
	std::vector<std::string> tokens;
	unsigned int index;
};

#endif /* _TOKENIZER_H */

