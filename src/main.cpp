#include "stdafx.h"
#include "PointcloudVisualizer.h"
#include "PCDparser.h"
#include "StringUtils.h"


int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		std::cerr << "ERROR! Arguments must be of the format: 'PointcloudVisualizer.exe [pointcloud file name]'." << std::endl;
		std::cerr << "Files may be in the following formats: pcd, csv, depth images (jpg/jpeg,png,bmp,ppm,tiff)" << std::endl;
	}

	PointcloudVisualizer::PointcloudVisualizer pcv;
	pcv.initialize(1280, 720);

	// Get filename, convert to lowercase, parse filetype.
	std::string pointcloudFilename = argv[1];
	for (unsigned int i = 0; i < pointcloudFilename.size(); ++i)
	{
		pointcloudFilename[i] = std::tolower(pointcloudFilename[i]);
	}

	std::string extension = pointcloudFilename.substr(pointcloudFilename.rfind("."));


	// Load data by format.
	if (extension == ".pcd")
	{
		PCDparser::PCDparser pcdParser(pointcloudFilename);
		pcv.addData(pcdParser.data);
	}

	else if (extension == ".csv") // Handle ascii CSV text data
	{
		std::ifstream infile(pointcloudFilename, std::ios::binary | std::ios::in);
		std::string inputBuffer = "";
		std::vector<std::vector<float>> data;

		std::vector<float> currentPoint;
		while (std::getline(infile, inputBuffer))
		{
			std::vector<std::string> parsedLine = PointcloudVisualizer::tokenize(inputBuffer, ",");

			for (unsigned int i = 0; i < parsedLine.size(); ++i)
			{
				if (currentPoint.size() == 3)
				{
					data.push_back(currentPoint);
					currentPoint.clear();
				}
				currentPoint.push_back(std::stof(parsedLine[i]));
			}
		}

		infile.close();
		pcv.addData(data);
		data.clear();
	}
	else // Default behavior, handle greyscale image files (to be read using OpenCV's codecs)
	{
		cv::Mat data1;
		data1 = cv::imread(pointcloudFilename, cv::IMREAD_GRAYSCALE);
		pcv.addData(data1);
	}

	pcv.meshes[0].scale = glm::vec3(1);
	pcv.meshes[0].rotation = glm::vec3(0);
	pcv.meshes[0].position = glm::vec3(0,0,-20);

	pcv.RenderLoop();

	return 0;
}







