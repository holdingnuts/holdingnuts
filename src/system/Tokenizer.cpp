#include "Tokenizer.hpp"

using namespace std;

bool isSep(char ch, string sep)
{
	for (unsigned int i=0; i < sep.length(); i++)
	{
		if (sep[i] == ch)
			return true;
	}
	
	return false;
}

bool Tokenizer::parse(string str, string sep)
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
			int token_end;
			
			if (!quote_open)
			{
				if (i == str.length()-1)
				{
					end_tok = true;
					token_end = i - token_start + 1;
				}
				else if (isSep(cur_char, sep))
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
			if (!isSep(cur_char, sep))
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
					token_start = i;
			}
		}
		
		last_char = cur_char;
	}
	
	return true;
}

bool Tokenizer::getNext(string &str)
{
	if (index == tokens.size())
		return false;
	
	str = tokens[index++];
	return true;
}
