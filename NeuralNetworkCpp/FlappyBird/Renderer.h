#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Graphics.h"
#include <stdexcept>
#include "implot.h"
#include "glm.hpp"
#include "Texture.h"
#include "ext/matrix_transform.hpp"
#include "ext/matrix_clip_space.hpp"
//this is just a basic renderer literally made for flappy bird
//yeeah no batching or nothing, it really sucks
class Renderer
{
public:
	void SetupWindow(GLuint width, GLuint height, const char* title);
	void SetupImgui();
	void StartFrame();
	void EndFrame();
	void UnSetup();

	void DrawSprite(Texture* texture, glm::vec2 position, float width, float height, float rotation = 0.0f, glm::vec3 colour = glm::vec3(1, 1, 1), glm::vec2 pivot = glm::vec2(0.5f, 0.5f));
	void DrawBox(glm::vec2 position, float width, float height, float rotation = 0.0f, glm::vec3 colour = glm::vec3(1, 1, 1), glm::vec2 pivot = glm::vec2(0.5f, 0.5f));

	struct Camera
	{
		glm::vec2 position;
		float rotation;
		float size;
		float aspect;
	private:
		friend Renderer;

		glm::mat4x4 projection;
		glm::mat4x4 view;
	};

	static constexpr int BATCH_SIZE = 50;
	inline const Camera& GetCamera() { return camera; }
	inline GLFWwindow* GetWindow() { return window; }

	//also updates the projection matrix
	inline void SetCameraRotation(float rotation);
	inline void SetCameraPosition(glm::vec2 position);
	inline void SetCameraSize(float size);

private:
	void UpdateCamera();

	GLFWwindow* window;
	Camera camera;

	Texture defaultTexture;
	//quad mesh data
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLuint elementBuffer;
	//shader program id
	GLuint shaderID;
	//uniform locations
	GLuint colourLocation;
	GLuint transformLocation;
	GLuint viewProjectionLocation;
};

