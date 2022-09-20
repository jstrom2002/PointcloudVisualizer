/*!
*	PointcloudVisualizer.h -- Pointcloud visualization header using GLFW3.
*/

#ifndef POINTCLOUD_VISUALIZER_H
#define POINTCLOUD_VISUALIZER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <opencv2/opencv.hpp>


namespace PointcloudVisualizer
{
	// Externs that cannot be placed internally within a class.
	extern void cursorCallback(GLFWwindow* window, double xpos, double ypos);
	extern void APIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

	extern float lastX;
	extern float lastY;
	extern bool firstMouse;
	extern float deltaTime;
	extern float lastFrame;
	extern float movementSpeed;

	/*!
	*  \brief Returns a matrix that is transformed given vec3 for rotation.
	*/
	glm::mat4 rotateMatrix(glm::vec3 rot,glm::mat4 mat);

	/*!
	*  \brief Returns a matrix that is transformed given vec3's for translation, scale, and rotation
	*/
	glm::mat4 transformMatrix(glm::vec3 pos = glm::vec3(0.0f),glm::vec3 scale = glm::vec3(1.0f),glm::vec3 rot = glm::vec3(0.0f),glm::mat4 mat = glm::mat4(1.0f));



	// Camera class
	enum class Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	class Camera{
	public:
		// Camera Attributes
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;
		// Eular Angles
		GLfloat Yaw;
		GLfloat Pitch;
		// Camera options
		GLfloat MovementSpeed;
		GLfloat MouseSensitivity;
		GLfloat Zoom;

		// Constructor with vectors
		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, -3.0f),
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
			GLfloat yaw = -90.0f, GLfloat pitch = 0) :
			Front(glm::vec3(0.0f, 0.0f, 1.0f)), MovementSpeed(3.5), MouseSensitivity(0.1), Zoom(45.0){
			this->Position = position;
			this->WorldUp = up;
			this->Yaw = yaw;
			this->Pitch = pitch;
			this->updateCameraVectors();
		}

		// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
		glm::mat4 GetViewMatrix(){return glm::lookAt(this->Position, this->Position + this->Front, this->Up);}

		// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
		void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime){
			GLfloat velocity = this->MovementSpeed * deltaTime;
			if (direction == Camera_Movement::FORWARD)
				this->Position += this->Front * velocity;
			if (direction == Camera_Movement::BACKWARD)
				this->Position -= this->Front * velocity;
			if (direction == Camera_Movement::LEFT)
				this->Position -= this->Right * velocity;
			if (direction == Camera_Movement::RIGHT)
				this->Position += this->Right * velocity;
		}

		// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
		void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true){
			xoffset *= this->MouseSensitivity;
			yoffset *= this->MouseSensitivity;

			this->Yaw += xoffset;
			this->Pitch += yoffset;

			// Make sure that when pitch is out of bounds, screen doesn't get flipped
			if (constrainPitch)
			{
				if (this->Pitch > 89.0f)
					this->Pitch = 89.0f;
				if (this->Pitch < -89.0f)
					this->Pitch = -89.0f;
			}

			// Update Front, Right and Up Vectors using the updated Eular angles
			this->updateCameraVectors();
		}

		// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void ProcessMouseScroll(GLfloat yoffset){
			if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
				this->Zoom -= yoffset;
			if (this->Zoom <= 1.0f)
				this->Zoom = 1.0f;
			if (this->Zoom >= 45.0f)
				this->Zoom = 45.0f;
		}

	private:
		// Calculates the front vector from the Camera's (updated) Eular Angles
		void updateCameraVectors(){
			// Calculate the new Front vector
			glm::vec3 front;
			front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
			front.y = sin(glm::radians(this->Pitch));
			front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
			this->Front = glm::normalize(front);
			// Also re-calculate the Right and Up vector
			this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			this->Up = glm::normalize(glm::cross(this->Right, this->Front));
		}
	};

	extern Camera camera;

	class Shader{
	public:
		unsigned int ID;
		Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);		
		void use() { glUseProgram(ID); }
		void setBool(const std::string& name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); }
		void setInt(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
		void setFloat(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
		void setVec2(const std::string& name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
		void setVec2(const std::string& name, float x, float y) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); }
		void setVec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
		void setVec3(const std::string& name, float x, float y, float z) const { glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); }
		void setVec4(const std::string& name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
		void setVec4(const std::string& name, float x, float y, float z, float w) { glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); }
		void setMat2(const std::string& name, const glm::mat2& mat) const { glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
		void setMat3(const std::string& name, const glm::mat3& mat) const { glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
		void setMat4(const std::string& name, const glm::mat4& mat) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }

	private:
		void checkCompileErrors(GLuint shader, std::string type);
	};

	class PointcloudVisualizer {
	public:
		class CloudMesh {
		public:
			enum class DATA_TYPE {
				CV,
				STL,
				GLM
			};

			DATA_TYPE datatype;
			cv::Mat dataCV;
			std::vector<std::vector<float>> dataSTL;
			std::vector<glm::vec3> dataGLM;
			//af::array dataAF;//currently, OpenGL has issues with Arrayfire's JIT compiler and won't work.
			unsigned int VAO, VBO, VBO2, drawCount;
			glm::vec3 position;
			glm::vec3 scale;
			glm::vec3 rotation;
			glm::vec3 cloud_color;

			/*!
			*  \brief OpenCV Mat initializer.
			*/
			CloudMesh(cv::Mat& data_) : 
				VAO(0), VBO(0), VBO2(0), drawCount(0), 
				position(glm::vec3(0)), scale(glm::vec3(1)), rotation(glm::vec3(0)),
				cloud_color(glm::vec3(1)), datatype(DATA_TYPE::CV), dataCV(data_)
			{}

			/*!
			*  \brief stl vector matrix initializer.
			*/
			CloudMesh(std::vector<std::vector<float>>& data_) :
				VAO(0), VBO(0), VBO2(0), drawCount(0),
				position(glm::vec3(0)), scale(glm::vec3(1)), rotation(glm::vec3(0)),
				cloud_color(glm::vec3(1)), datatype(DATA_TYPE::STL), dataSTL(data_)
			{}

			/*!
			*  \brief glm vector matrix initializer.
			*/
			CloudMesh(std::vector<glm::vec3>& data_) :
				VAO(0), VBO(0), VBO2(0), drawCount(0),
				position(glm::vec3(0)), scale(glm::vec3(1)), rotation(glm::vec3(0)),
				cloud_color(glm::vec3(1)), datatype(DATA_TYPE::GLM), dataGLM(data_)
			{}

			/*!
			*  \brief Loads data at render-time.
			*/
			void loadVAO();

			/*!
			*  \brief Deletes all buffers and arrays stored on the GPU.
			*/
			void clear();

			float width() {return std::abs(x_max - x_min);}
			float height() {return std::abs(y_max - y_min);}
			float depth() {return std::abs(z_max - z_min);}

		private:
			/*!
			*  \brief Dimensions of bounding box around the point cloud mesh.
			*/
			float x_min, x_max, y_min, y_max, z_min, z_max;

			/*!
			*  \brief Loads data for opencv Mats.
			*/
			void loadVAO_CV();

			/*!
			*  \brief Loads data for stl std::vector<std::vector<float>> arrays.
			*/
			void loadVAO_STL();

			/*!
			*  \brief Loads data for stl std::vector<glm::vec3> arrays.
			*/
			void loadVAO_GLM();
		};


		std::vector<CloudMesh> meshes;		
		char keyTimer;
		unsigned int axisVAO, axisVBO, axisVBO2;
		glm::vec3 lightPos = glm::vec3(0);
		Shader* shader;
		GLFWwindow* window;


		void processInput(GLFWwindow* window);

		~PointcloudVisualizer();
		PointcloudVisualizer() {}

		void clear();	

		void shutdown();

		void initialize(int window_width, int window_height);

		void addData(cv::Mat& cloud);

		void addData(std::vector<std::vector<float>>& cloud);
	
		void addData(std::vector<glm::vec3>& cloud);


		void Draw() {
			// Check to see if there is anything saved to draw.
			if (this->meshes.size() <= 0) { return; }

			// Set all necessary global GL states.
			glDisable(GL_CULL_FACE);
			glCullFace(GL_CCW);
			glEnable(GL_DEPTH_TEST);

			// Set shader program and set uniforms for shader.
			shader->use();
			shader->setMat4("view", camera.GetViewMatrix());
			shader->setVec3("viewPos", camera.Position);
			shader->setVec3("lightPos", this->lightPos);

			// Draw all saved cloud meshes.
			for (int i = 0; i < meshes.size(); ++i) {
				meshes[i].loadVAO();
				shader->setMat4("model", transformMatrix(meshes[i].position, meshes[i].scale, meshes[i].rotation));
				shader->setVec3("cloud_color", this->meshes[i].cloud_color);
				glBindVertexArray(meshes[i].VAO);
				glDrawArrays(GL_POINTS, 0, meshes[i].drawCount);
			}

			glBindVertexArray(0);
		}

		void RenderLoop();

		private:
			int window_width, window_height;
			void saveFramebufferToFile(GLuint buff=0, std::string filename="", std::string format="JPG");
	};
}// END NAMESPACE
#endif