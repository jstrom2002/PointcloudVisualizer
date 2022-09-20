#include "stdafx.h"
#include "PCDparser.h"
#include <fstream>

namespace PCDparser
{
	PCDparser::PCDparser(std::string filename)
	{
		std::ifstream infile(filename, std::ios::binary | std::ios::in);
		std::string inputBuffer = "";
		bool parsingHeader = true;

		while (std::getline(infile, inputBuffer))
		{
			// Skip commented lines (denoted by a # char).
			if (inputBuffer.size() > 0 && inputBuffer[0] == '#')
				continue;

			std::string firstEntry = inputBuffer.substr(0, inputBuffer.find(" "));
			if (IsHeaderString(firstEntry) < 0)
			{
				parsingHeader = false;
			}

			std::vector<std::string> parsedLine = tokenize(inputBuffer, " ");
			std::vector<float> currentPoint;

			if (parsingHeader)
			{
				// Handle header entries. TO DO: implement
			}
			else
			{
				for (unsigned int i = 0; i < parsedLine.size(); ++i)
					currentPoint.push_back(std::stof(parsedLine[i]));

				data.push_back(currentPoint);
			}
		}
		
		infile.close();
	}

	int PCDparser::IsHeaderString(std::string entry)
	{
		for (unsigned int i = 0; i < HEADERS; ++i)
		{
			if (entry == HeaderEntry[i])
				return i;
		}

		return -1;
	}

	std::vector<std::string> PCDparser::tokenize(std::string toTokenize, std::string token) {
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