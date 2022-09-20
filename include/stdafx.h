#pragma once

// Dependencies: glad, OpenCV 4, GLEW, GLFW, GLM

# pragma comment(lib, "glad.lib")
# pragma comment(lib, "zlibd.lib")
# pragma comment(lib, "OpenGL32.lib")
# pragma comment(lib, "glew32d.lib")
# pragma comment(lib, "glfw3.lib")
# pragma comment(lib, "jpeg.lib")
# pragma comment(lib, "turbojpeg.lib")
# pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#pragma comment(lib, "bz2d.lib")
#pragma comment(lib, "libcurl-d.lib")
#pragma comment(lib, "glew32d.lib")
#pragma comment(lib, "libprotobufd.lib")
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "tiffd.lib")
#pragma comment(lib, "webpd.lib")
#pragma comment(lib, "IlmImf-2_5_d.lib")
#pragma comment(lib, "Iex-2_5_d.lib")
#pragma comment(lib, "IlmThread-2_5_d.lib")
#pragma comment(lib, "Imath-2_5_d.lib")
#pragma comment(lib, "Half-2_5_d.lib")
#pragma comment(lib, "lzmad.lib")
#pragma comment(lib, "libszip_D.lib")
#pragma comment(lib, "libhdf5_D.lib")
#pragma comment(lib, "lzmad.lib")
#pragma comment(lib, "lz4d.lib")
#pragma comment(lib, "libpng16d.lib")
#pragma comment(lib, "opencv_cored.lib")
#pragma comment(lib, "opencv_highguid.lib")
#pragma comment(lib, "opencv_imgcodecsd.lib")
#pragma comment(lib, "opencv_imgprocd.lib")
#else
#pragma comment(lib, "bz2.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "tiff.lib")
#pragma comment(lib, "webp.lib")
#pragma comment(lib, "IlmImf-2_5.lib")
#pragma comment(lib, "IlmThread-2_5.lib")
#pragma comment(lib, "Imath-2_5.lib")
#pragma comment(lib, "Half-2_5.lib")
#pragma comment(lib, "Iex-2_5.lib")
#pragma comment(lib, "lzma.lib")
#pragma comment(lib, "lz4.lib")
#pragma comment(lib, "libpng16.lib")
#pragma comment(lib, "libszip.lib")
#pragma comment(lib, "libhdf5.lib")
#pragma comment(lib, "opencv_core.lib")
#pragma comment(lib, "opencv_highgui.lib")
#pragma comment(lib, "opencv_imgcodecs.lib")
#pragma comment(lib, "opencv_imgproc.lib")
#endif

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>