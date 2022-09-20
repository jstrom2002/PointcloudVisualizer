#include "stdafx.h"
#include "PointcloudVisualizer.h"
#include "PCDparser.h"


int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		std::cerr << "ERROR! Arguments must be of the format: 'PointcloudVisualizer.exe [pointcloud file name]'." << std::endl;
		std::cerr << "Files may be in the following formats: jpg/jpeg,png,bmp,ppm,tiff,pcd" << std::endl;
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
	if (extension == ".jpg" || // Handle image files (to be read using OpenCV's codecs)
		extension == ".jpeg"||
		extension == ".bmp" ||
		extension == ".png" ||
		extension == ".ppm" ||
		extension == ".tiff")
	{
		cv::Mat data1;
		data1 = cv::imread(pointcloudFilename, cv::IMREAD_UNCHANGED);
		pcv.addData(data1);
	}

	else if (extension == ".pcd")
	{
		PCDparser::PCDparser pcdParser(pointcloudFilename);
		pcv.addData(pcdParser.data);
	}

	else // Handle ascii text data
	{

		// TO DO: handle text data
	}


	pcv.meshes[0].scale = glm::vec3(1);
	pcv.meshes[0].rotation = glm::vec3(0);
	pcv.meshes[0].position = glm::vec3(0,0,-20);

	pcv.RenderLoop();

	return 0;
}







