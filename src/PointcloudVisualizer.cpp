#include "PointcloudVisualizer.h"
#include <algorithm>
#include <gl/glext.h>

namespace PointcloudVisualizer 
{
	// Resolve all externs required by PointcloudVisualizer class here.
	float lastX = 800.0f / 2.0;
	float lastY = 600.0 / 2.0;
	bool firstMouse = true;
	float deltaTime = 0.0f;	// time between current frame and last frame
	float lastFrame = 0.0f;
	float movementSpeed = 2.5;
	Camera camera;
}

void PointcloudVisualizer::Shader::checkCompileErrors(GLuint shader, std::string type) {
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog <<
				"\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog <<
				"\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}


PointcloudVisualizer::Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
	// Retrieve the vertex/fragment source code from filePath.
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// if geometry shader path is present, also load a geometry shader
		if (geometryPath != nullptr)
		{
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if (geometryPath != nullptr)
	{
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	if (geometryPath != nullptr)
		glAttachShader(ID, geometry);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
		glDeleteShader(geometry);

}


void APIENTRY PointcloudVisualizer::debugMessageCallback(
	GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam){
	// Some debug messages are just annoying informational messages
	switch (id)
	{
	case 131185: // glBufferData
		return;
	}

	printf("Message: %s\n", message);
	printf("Source: ");

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		printf("API");
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		printf("Window System");
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		printf("Shader Compiler");
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		printf("Third Party");
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		printf("Application");
		break;
	case GL_DEBUG_SOURCE_OTHER:
		printf("Other");
		break;
	}

	printf("\n");
	printf("Type: ");

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		printf("Error");
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		printf("Deprecated Behavior");
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		printf("Undefined Behavior");
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		printf("Portability");
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		printf("Performance");
		break;
	case GL_DEBUG_TYPE_MARKER:
		printf("Marker");
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		printf("Push Group");
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		printf("Pop Group");
		break;
	case GL_DEBUG_TYPE_OTHER:
		printf("Other");
		break;
	}

	printf("\n");
	printf("ID: %d\n", id);
	printf("Severity: ");

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		printf("High");
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		printf("Medium");
		break;
	case GL_DEBUG_SEVERITY_LOW:
		printf("Low");
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		printf("Notification");
		break;
	}

	printf("\n\n");
}

void PointcloudVisualizer::PointcloudVisualizer::addData(cv::Mat& cloud) {
	// Convert to single-channel, grayscale image if necessary.
	if (cloud.type() != CV_32F) {
		if (cloud.channels() > 1) {
			cv::cvtColor(cloud, cloud, cv::COLOR_BGR2GRAY);
		}
		if (cloud.type() != CV_32F) {
			cloud.convertTo(cloud, CV_32F);
		}
	}

	// Add object to data array.
	CloudMesh mesh(cloud);
	this->meshes.push_back(mesh);
}

void PointcloudVisualizer::PointcloudVisualizer::addData(std::vector<glm::vec3>& cloud) 
{
	CloudMesh mesh(cloud);
	this->meshes.push_back(mesh);
}

void PointcloudVisualizer::PointcloudVisualizer::addData(std::vector<std::vector<float>>& cloud) 
{
	CloudMesh mesh(cloud);
	this->meshes.push_back(mesh);
}

void PointcloudVisualizer::cursorCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

PointcloudVisualizer::PointcloudVisualizer::~PointcloudVisualizer() 
{
	shutdown();
}

void PointcloudVisualizer::PointcloudVisualizer::shutdown() 
{
	clear();
	delete shader;
}

void PointcloudVisualizer::PointcloudVisualizer::clear() 
{
	for (int i = 0; i < meshes.size(); ++i)
		meshes[i].clear();	

	if (this->axisVAO)
		glDeleteVertexArrays(1, &axisVAO);
	if (this->axisVBO) 
		glDeleteBuffers(1, &axisVBO);
	if (this->axisVBO2) 
		glDeleteBuffers(1, &axisVBO2);
}

void PointcloudVisualizer::PointcloudVisualizer::initialize(int w, int h) 
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	window = glfwCreateWindow(w, h, "Pointcloud Visualizer", NULL, NULL);	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwMakeContextCurrent(window);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	window_width = w;
	window_height = h;

	//// Check to see if debug context is available.
	//GLint flags;
	//glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	//try {
	//	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
	//		glEnable(GL_DEBUG_OUTPUT);
	//		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//		glDebugMessageCallback(debugMessageCallback, NULL);
	//		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	//	}
	//}
	//catch (std::exception e1) {
	//	std::cerr << e1.what() << std::endl;
	//}

	glfwSetFramebufferSizeCallback(window,[](GLFWwindow* window, int width, int height) {glViewport(0, 0, width, height);});

	glfwSetCursorPosCallback(window, (GLFWcursorposfun)cursorCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	glEnable(GL_PROGRAM_POINT_SIZE);

	// Setup shader
	shader = new Shader("shader.vs", "shader.fs");
	shader->use();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	shader->setMat4("projection", projection);
	shader->setInt("texture1", 0);


	// Load VAO for axis.
	float points[] = {
		-0.5, 0.01, -0.5,  0.0, 1.0, 0.0,
		 0.5, 0.01, -0.5,  0.0, 1.0, 0.0,
		-0.5, 0.01,  0.5,  0.0, 1.0, 0.0,

		 0.5, 0.01, -0.5,  0.0, 1.0, 0.0,
		-0.5, 0.01,  0.5,  0.0, 1.0, 0.0,
		-0.5, 0.01,  0.5,  0.0, 1.0, 0.0,

		-0.5, -0.01, -0.5,  0.0, -1.0, 0.0,
		-0.5, -0.01,  0.5,  0.0, -1.0, 0.0,
		 0.5, -0.01, -0.5,  0.0, -1.0, 0.0,

		-0.5, -0.01, -0.5,  0.0, -1.0, 0.0,
		-0.5, -0.01,  0.5,  0.0, -1.0, 0.0,
		-0.5, -0.01,  0.5,  0.0, -1.0, 0.0
	};
	glGenVertexArrays(1, &this->axisVAO);
	glBindVertexArray(this->axisVAO);

	// aPos
	glGenBuffers(1, &this->axisVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->axisVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// aNormal
	glGenBuffers(1, &this->axisVBO2);
	glBindBuffer(GL_ARRAY_BUFFER, this->axisVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void PointcloudVisualizer::PointcloudVisualizer::processInput(GLFWwindow* window){
	if (keyTimer <= 0) { keyTimer = 0; }
	else { keyTimer--; }

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	else if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS)
		camera.Position = glm::vec3(0);
	else if (glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS && keyTimer == 0) {//screenshot button
		saveFramebufferToFile(0);
		keyTimer = 50;
	}
	else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		float cameraSpeed = movementSpeed * deltaTime;
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		float cameraSpeed = movementSpeed * deltaTime;
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		float cameraSpeed = movementSpeed * deltaTime;
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		float cameraSpeed = movementSpeed * deltaTime;
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	}
}

void PointcloudVisualizer::PointcloudVisualizer::CloudMesh::loadVAO_CV() {
	// Check if this pointcloud mesh has been loaded, and if so return.
	if (dataCV.empty() || VAO || VBO || VBO2) { return; }

	std::vector<glm::vec3> transVecs;
	std::vector<glm::vec3> normals;

	x_min = y_min = z_min = 99999999;
	x_max = y_max = z_max = -99999999;

	// Save all point coordinates on the GPU.
	for (int i = 0; i < dataCV.rows - 1; ++i) {
		for (int j = 0; j < dataCV.cols - 1; ++j) {

			if (j < x_min) { x_min = j; }
			if (j > x_max) { x_max = j; }
			if (i < y_min) { y_min = i; }
			if (i > y_max) { y_max = i; }

			float d1 = dataCV.at<float>(i, j);
			float d2 = dataCV.at<float>(i, j + 1);
			float d3 = dataCV.at<float>(i + 1, j);
			float d4 = dataCV.at<float>(i + 1, j + 1);
			std::vector<float> vec{ d1, d2, d3, d4 };
			float dmin = *std::min_element(vec.begin(), vec.end());
			float dmax = *std::max_element(vec.begin(), vec.end());

			if (dmin < z_min) { z_min = dmin; }
			if (dmax > z_max) { z_max = dmax; }

			transVecs.push_back(glm::vec3(i, j, d1));
			transVecs.push_back(glm::vec3(i, j + 1, d2));
			transVecs.push_back(glm::vec3(i + 1, j, d3));

			transVecs.push_back(glm::vec3(i, j + 1, d2));
			transVecs.push_back(glm::vec3(i + 1, j + 1, d4));
			transVecs.push_back(glm::vec3(i + 1, j, d3));
		}
	}

	// Calculate normals.
	for (int i = 0; i < transVecs.size(); i += 3) {
		glm::vec3 normal =
			glm::normalize(
				glm::cross(
					glm::vec3(transVecs[i + 1]) - glm::vec3(transVecs[i]),
					glm::vec3(transVecs[i + 2]) - glm::vec3(transVecs[i])
				)
			);
		normals.push_back(normal);
	}

	unsigned int VAO_, VBO_, VBO2_, drawCount_;
	VAO_ = VBO_ = VBO2_ = drawCount_ = 0;

	// Load into VAO
	glBindVertexArray(VAO_);
	glGenVertexArrays(1, &VAO_);
	glBindVertexArray(VAO_);

	// aPos
	glGenBuffers(1, &VBO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, transVecs.size() * sizeof(glm::vec3), &transVecs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	// aNormal
	glGenBuffers(1, &VBO2_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2_);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(1);

	VAO = VAO_;
	VBO = VBO_;
	//cl.VBO2 = VBO2_;
	drawCount = transVecs.size();

	transVecs.clear();
	//normals.clear();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PointcloudVisualizer::PointcloudVisualizer::CloudMesh::loadVAO_GLM() {
	// Check if this pointcloud mesh has been loaded, and if so return.
	if (dataGLM.size() == 0 || VAO || VBO || VBO2) { return; }

	std::vector<glm::vec3> transVecs;
	std::vector<glm::vec3> normals;

	x_min = y_min = z_min = 99999999;
	x_max = y_max = z_max = -99999999;

	// Save all point coordinates on the GPU.
	for (int i = 0; i < dataGLM.size()-3; ++i) {

			if (dataGLM[i].x < x_min) { x_min = dataGLM[i].x; }
			if (dataGLM[i].x > x_max) { x_max = dataGLM[i].x; }
			if (dataGLM[i].y < y_min) { y_min = dataGLM[i].y; }
			if (dataGLM[i].y > y_max) { y_max = dataGLM[i].y; }

			float d1 = dataGLM[i].z;
			float d2 = dataGLM[i + 1].z;
			float d3 = dataGLM[i + 2].z;
			float d4 = dataGLM[i + 3].z;
			std::vector<float> vec{ d1, d2, d3, d4 };
			float dmin = *std::min_element(vec.begin(), vec.end());
			float dmax = *std::max_element(vec.begin(), vec.end());

			if (dmin < z_min) { z_min = dmin; }
			if (dmax > z_max) { z_max = dmax; }

			transVecs.push_back(dataGLM[i]);
			transVecs.push_back(dataGLM[i+1]);
			transVecs.push_back(dataGLM[i+2]);

			transVecs.push_back(dataGLM[i+1]);
			transVecs.push_back(dataGLM[i+3]);
			transVecs.push_back(dataGLM[i+2]);		
	}

	// Calculate normals.
	for (int i = 0; i < transVecs.size(); i += 3) {
		glm::vec3 normal =
			glm::normalize(
				glm::cross(
					glm::vec3(transVecs[i + 1]) - glm::vec3(transVecs[i]),
					glm::vec3(transVecs[i + 2]) - glm::vec3(transVecs[i])
				)
			);
		normals.push_back(normal);
	}

	unsigned int VAO_, VBO_, VBO2_, drawCount_;
	VAO_ = VBO_ = VBO2_ = drawCount_ = 0;

	// Load into VAO
	glBindVertexArray(VAO_);
	glGenVertexArrays(1, &VAO_);
	glBindVertexArray(VAO_);

	// aPos
	glGenBuffers(1, &VBO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, transVecs.size() * sizeof(glm::vec3), &transVecs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	// aNormal
	glGenBuffers(1, &VBO2_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2_);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(1);

	VAO = VAO_;
	VBO = VBO_;
	drawCount = transVecs.size();

	transVecs.clear();
	normals.clear();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PointcloudVisualizer::PointcloudVisualizer::CloudMesh::loadVAO_STL() 
{
	// Check if this pointcloud mesh has been loaded, and if so return.
	if (dataSTL.size() == 0 || VAO || VBO || VBO2) 
		return;

	std::vector<glm::vec3> transVecs;
	std::vector<glm::vec3> normals;

	x_min = y_min = z_min = 99999999;
	x_max = y_max = z_max = -99999999;

	// Save all point coordinates on the GPU.
	for (int i = 0; i < dataSTL.size() - 1; ++i) {
		for (int j = 0; j < dataSTL[0].size() - 1; ++j) {

			if (j < x_min) { x_min = j; }
			if (j > x_max) { x_max = j; }
			if (i < y_min) { y_min = i; }
			if (i > y_max) { y_max = i; }

			float d1 = dataSTL[i][j];
			float d2 = dataSTL[i][j + 1];
			float d3 = dataSTL[i + 1][j];
			float d4 = dataSTL[i + 1][j + 1];
			std::vector<float> vec{ d1, d2, d3, d4 };
			float dmin = *std::min_element(vec.begin(), vec.end());
			float dmax = *std::max_element(vec.begin(), vec.end());

			if (dmin < z_min) { z_min = dmin; }
			if (dmax > z_max) { z_max = dmax; }

			transVecs.push_back(glm::vec3(i, j, d1));
			transVecs.push_back(glm::vec3(i, j + 1, d2));
			transVecs.push_back(glm::vec3(i + 1, j, d3));

			transVecs.push_back(glm::vec3(i, j + 1, d2));
			transVecs.push_back(glm::vec3(i + 1, j + 1, d4));
			transVecs.push_back(glm::vec3(i + 1, j, d3));
		}
	}

	// Calculate normals.
	for (int i = 0; i < transVecs.size(); i += 3) {
		glm::vec3 normal =
			glm::normalize(
				glm::cross(
					glm::vec3(transVecs[i + 1]) - glm::vec3(transVecs[i]),
					glm::vec3(transVecs[i + 2]) - glm::vec3(transVecs[i])
				)
			);
		normals.push_back(normal);
	}

	unsigned int VAO_, VBO_, VBO2_, drawCount_;
	VAO_ = VBO_ = VBO2_ = drawCount_ = 0;

	// Load into VAO
	glBindVertexArray(VAO_);
	glGenVertexArrays(1, &VAO_);
	glBindVertexArray(VAO_);

	// aPos
	glGenBuffers(1, &VBO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, transVecs.size() * sizeof(glm::vec3), &transVecs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	// aNormal
	glGenBuffers(1, &VBO2_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2_);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(1);

	VAO = VAO_;
	VBO = VBO_;
	drawCount = transVecs.size();

	transVecs.clear();
	normals.clear();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PointcloudVisualizer::PointcloudVisualizer::CloudMesh::loadVAO() {
	switch (datatype) {
	case DATA_TYPE::CV:
		loadVAO_CV();
	case DATA_TYPE::STL:
		loadVAO_STL();
	case DATA_TYPE::GLM:
		loadVAO_GLM();
	}
}

void PointcloudVisualizer::PointcloudVisualizer::CloudMesh::clear(){
	if (VAO) { glDeleteVertexArrays(1, &VAO); }
	if (VBO) { glDeleteBuffers(1, &VBO); }
	if (VBO2) { glDeleteBuffers(1, &VBO2); }
}

glm::mat4 PointcloudVisualizer::rotateMatrix(glm::vec3 front,glm::mat4 mat) {
	mat = glm::rotate(
			glm::rotate(
				glm::rotate(
					mat,
					glm::radians(front.z),
					glm::vec3(0, 0, 1)
				),
				glm::radians(front.y),
				glm::vec3(0, 1, 0)
			),
			glm::radians(front.x),
			glm::vec3(1, 0, 0)
		);
	return mat;
}

glm::mat4 PointcloudVisualizer::transformMatrix(glm::vec3 pos,glm::vec3 scale,glm::vec3 rot,glm::mat4 mat) {
	mat = rotateMatrix(rot, mat);
	mat = glm::scale(mat, scale);
	mat = glm::translate(mat, pos);
	return mat;
}

void PointcloudVisualizer::PointcloudVisualizer::saveFramebufferToFile(GLuint buff, std::string filename, std::string format) {
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, buff);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	char cFileName[64];
	FILE* fScreenshot = NULL;
	int nSize = this->window_width * this->window_height * 3;
	if (nSize == 0) return;

	// read framebuffer data 
	std::vector<GLubyte> pixels;
	pixels.resize(nSize);
	glReadPixels(0, 0, window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

	// Save to JPG file
	// ----------------
	if (format == "JPG" || format == "JPEG") {

		if (filename == "") {
			//check and get next file name in sequence
			int nShot = 0;
			while (nShot < 500) {
				sprintf(cFileName, "screenshots/screenshot_%d.jpg", nShot);
				fScreenshot = fopen(cFileName, "rb");
				if (fScreenshot == NULL) break;
				else fclose(fScreenshot);
				++nShot;
				if (nShot > 499) {
					std::cout << "Screenshot limit of 500 reached.\n";
					return;
				}
			}
			if (fScreenshot) { fclose(fScreenshot); }
		}

		cv::Mat img(window_height, window_width, CV_8UC3);

		//use fast 4-byte alignment (default anyway) if possible
		glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

		//set length of one complete row in destination data (doesn't need to equal img.cols)
		glPixelStorei(GL_PACK_ROW_LENGTH, img.step / img.elemSize());

		//convert to BGR format    
		unsigned char temp;
		int i = 0;
		while (i < nSize) {
			temp = pixels[i];           //grab blue
			pixels[i] = pixels[i + 2];//assign red to blue
			pixels[i + 2] = temp;     //assign blue to red
			i += 3;     //skip to next blue byte
		}

		//glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);
		memcpy(img.data, pixels.data(), nSize);
		cv::flip(img, img, 0);


		if (filename == "") {
			cv::imwrite(cFileName, img);
		}
		else { cv::imwrite(filename, img); }

		img.deallocate();
	}

	// Save to TGA file
	// ----------------
	if (format == "TGA") {
		//check and get next file name in sequence
		int nShot = 0;
		while (nShot < 500) {
			sprintf(cFileName, "screenshots/screenshot_%d.tga", nShot);
			fScreenshot = fopen(cFileName, "rb");
			if (fScreenshot == NULL) break;
			else fclose(fScreenshot);
			++nShot;
			if (nShot > 499) {
				std::cout << "Screenshot limit of 500 reached.\n";
				return;
			}
		}

		fScreenshot = fopen(cFileName, "wb");

		//convert to BGR format    
		unsigned char temp;
		int i = 0;
		while (i < nSize) {
			temp = pixels[i];           //grab blue
			pixels[i] = pixels[i + 2];//assign red to blue
			pixels[i + 2] = temp;     //assign blue to red
			i += 3;     //skip to next blue byte
		}

		unsigned char TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
		unsigned char header[6] = { window_width % 256, window_width / 256, window_height % 256,window_height / 256,24,0 };
		fwrite(TGAheader, sizeof(unsigned char), 12, fScreenshot);
		fwrite(header, sizeof(unsigned char), 6, fScreenshot);
		fwrite(pixels.data(), sizeof(GLubyte), nSize, fScreenshot);
		fclose(fScreenshot);
	}
	pixels.clear();
	return;
}

void PointcloudVisualizer::PointcloudVisualizer::RenderLoop() {
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	clear();
}