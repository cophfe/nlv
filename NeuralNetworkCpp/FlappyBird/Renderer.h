#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Graphics.h"
#include <stdexcept>
#include "implot.h"
#include "glm.hpp"
#include "Texture.h"

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

	void DrawSprite(Texture* texture, glm::vec2 position, float width, float height, float rotation = 0.0f, glm::vec4 colour = glm::vec4(1, 1, 1, 1), glm::vec2 pivot = glm::vec2(0.5f, 0.5f));
	void DrawBox(glm::vec2 position, float width, float height, float rotation = 0.0f, glm::vec4 colour = glm::vec4(1, 1, 1, 1), glm::vec2 pivot = glm::vec2(0.5f, 0.5f));

	struct Camera
	{
		glm::vec2 position;
		float rotation;
		float scale;
	private:
		glm::mat4x4 projection;
	} camera;

	static constexpr int BATCH_SIZE = 50;
	inline const Camera& GetCamera() { return camera; }
	inline GLFWwindow* GetWindow() { return window; }

	//also updates the projection matrix
	inline void SetCameraRotation(float rotation);
	inline void SetCameraPosition(glm::vec2 position);
	inline void SetCameraScale(float scale);

private:
	
	GLFWwindow* window;

	//quad mesh data
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLuint elementBuffer;
	//shader program id
	GLuint shaderID;
};

