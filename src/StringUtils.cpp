#include "StringUtils.h"

namespace PointcloudVisualizer
{
	std::vector<std::string> tokenize(std::string toTokenize, std::string token) 
	{
		std::vector<std::string> result;
		char* tk = strtok((char*)toTokenize.c_str(), token.c_str());

		// Keep printing tokens while one of delimiters are present. 
		while (tk != NULL)
		{
			result.push_back(tk);
			tk = strtok(NULL, token.c_str());
		}

		return result;
	}
}