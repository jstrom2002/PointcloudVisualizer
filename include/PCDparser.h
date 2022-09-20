/*
*	PCDparser.h -- Class for parsing ASCII text .pcd files into an array of float 3-vecs.
*/

#pragma once
#include "stdafx.h"
#include <vector>

#define HEADERS 10u

namespace PCDparser
{
	class PCDparser
	{
	public:

		int width = 0;
		int height = 0;		
		std::vector<std::vector<float>> data;
	
		PCDparser(std::string filename);

	private:

		const std::string HeaderEntry[HEADERS]
		{
			"VERSION",
			"FIELDS",
			"SIZE",
			"TYPE",
			"COUNT",
			"WIDTH",
			"HEIGHT",
			"VIEWPOINT",
			"POINTS",
			"DATA"
		};

		int IsHeaderString(std::string entry);

		std::vector<std::string> tokenize(std::string toTokenize, std::string token);
	};
}