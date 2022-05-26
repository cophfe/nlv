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
class Renderer
{
public:
	void SetupWindow(GLuint width, GLuint height, const char* title);
	void SetupImgui();
	void StartFrame();
	void EndFrame();
	void UnSetup();

	//aie bootstrap renderer eat your heart out
	void DrawSprite(Texture* texture, glm::vec2 position, float width, float height, float rotation = 0.0f, glm::vec2 pivot = glm::vec2(0.5f, 0.5f), float depth = 0.0f);


	struct Camera
	{
		glm::vec2 position;
		float rotation;
		float scale;
	private:
		glm::mat4x4 projection;
	} camera;

	static constexpr int BATCH_SIZE = 50;
	inline Camera& GetCamera() { return camera; }
	inline GLFWwindow* GetWindow() { return window; }

private:
	
	GLFWwindow* window;

	//for sprite quads
	glm::vec3 vertex[BATCH_SIZE * 4];
	glm::vec2 indices[BATCH_SIZE * 6]
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLuint elementBuffer;
};

