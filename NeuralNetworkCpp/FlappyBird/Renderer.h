#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad.h"
#include "glfw3.h"
#include <stdexcept>
#include "implot.h"

class Renderer
{
public:
	void Setup(GLuint width, GLuint height, const char* title);
	void StartFrame();
	void EndFrame();
	void UnSetup();

	inline GLFWwindow* GetWindow() { return window; }

private:
	GLFWwindow* window;
};

