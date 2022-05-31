#include "Renderer.h"
#include <iostream>

void Renderer::SetupWindow(GLuint width, GLuint height, const char* title)
{
	if (glfwInit() == GL_FALSE)
		throw std::runtime_error("Something went wrong!");

	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		throw std::runtime_error("Something went wrong!");
	}
	glfwMaximizeWindow(window);

	glfwMakeContextCurrent(window);
	if (!gladLoadGL())
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		throw std::runtime_error("Something went wrong!");
	}

	glfwSwapInterval(0);
	glClearColor(0, 0, 0, 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	//FRAGMENT CREATE
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragmentString =
		R"(
#version 460 core
in vec2 TexCoord;
out vec4 Colour;
uniform sampler2D _Texture;
uniform vec3 _Colour;
void main()
{
    Colour = vec4(_Colour, 1.0) * textureLod(_Texture, TexCoord, 0);
    //Colour = ;
})";
	glShaderSource(fragment, 1, &fragmentString, nullptr);
	glCompileShader(fragment);

	GLint success = 0;
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar log[512];
		glGetShaderInfoLog(fragment, 512, nullptr, log);
		std::cout << "Failed to recompile frag shader:\n" << log << std::endl;
	}
	//VERTEX CREATE
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexString =
		R"(
#version 460 core
layout (location = 0) in vec2 position;

out vec2 TexCoord;
uniform mat4 _Model;
uniform mat4 _ViewProjection;
void main()
{
	TexCoord = position;
	gl_Position = _ViewProjection * (_Model * vec4(position - vec2(0.5), 1.0, 1.0));
})";
	glShaderSource(vertex, 1, &vertexString, nullptr);
	glCompileShader(vertex);

	success = 0;
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar log[512];
		glGetShaderInfoLog(vertex, 512, nullptr, log);
		std::cout << "Failed to recompile vert shader:\n" << log << std::endl;
	}
	//PROGRAM CREATE
	shaderID = glCreateProgram();
	glAttachShader(shaderID, vertex);
	glAttachShader(shaderID, fragment);
	glLinkProgram(shaderID);

	success = 0;
	glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLchar log[512];
		glGetProgramInfoLog(shaderID, 512, NULL, log);
		std::cout << "Failed to link shader program:\n" << log << std::endl;
	}
	//UNIFORM FIND
	
	glUseProgram(shaderID);
	colourLocation = glGetUniformLocation(shaderID, "_Colour");
	transformLocation = glGetUniformLocation(shaderID, "_Model");
	viewProjectionLocation = glGetUniformLocation(shaderID, "_ViewProjection");
	glUniform1ui(glGetUniformLocation(shaderID, "_Texture"), 0);
	//GET QUAD
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glm::vec2 vertices[4] = {
		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1}
	};
	//both local pos and uv data
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 4, (void*)vertices, GL_STATIC_DRAW);

	unsigned int indices[6] = {
		0, 1, 2,
		0, 2, 3
	};
	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), (void*)indices, GL_STATIC_DRAW);
	//define position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);


	defaultTexture.Load("white.png");

	//set up camera
	SetCameraSize(10.0f);
	SetCameraPosition(glm::vec3(0, 0, 0));
	SetCameraRotation(0);
	UpdateCamera();
}

void Renderer::SetupImgui()
{
	//Set up imgui (https://github.com/ocornut/imgui)
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
	
	//setup implot (https://github.com/epezent/implot)
	ImPlot::CreateContext();
}

void Renderer::StartFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	glClear(GL_COLOR_BUFFER_BIT);
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	
	UpdateCamera();
}

void Renderer::EndFrame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}

void Renderer::UnSetup()
{
	ImPlot::DestroyContext();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}

void Renderer::DrawSprite(Texture* texture, glm::vec2 position, float width, float height, float rotation, glm::vec3 colour, glm::vec2 pivot)
{
	//set colour
	glUniform3f(colourLocation, colour.x, colour.y, colour.z);
	//bind texture
	if (texture == nullptr)
		texture = &defaultTexture;
	texture->Bind();
	//calculate matrix
	rotation = glm::radians(rotation);
	glm::mat4 matrix = glm::identity<glm::mat4>();
	matrix = glm::translate(matrix, glm::vec3(position, 0.0f));
	matrix = glm::rotate(matrix, rotation, glm::vec3(0, 0, 1));
	//matrix = glm::translate(matrix, glm::vec3((pivot.x - 0.5f) * width, (pivot.y - 0.5f) * height, 0.0f));
	matrix = glm::scale(matrix, glm::vec3(width, height, 1));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, (GLfloat*)&(matrix[0]));
	//draw (batching you say? pffffffffffffffffffffff)
	glBindVertexArray(vertexArray);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawBox(glm::vec2 position, float width, float height, float rotation, glm::vec3 colour, glm::vec2 pivot)
{
	DrawSprite(&defaultTexture, position, width, height, rotation, colour, pivot);
}

inline void Renderer::SetCameraRotation(float rotation)
{
	camera.rotation = glm::radians(rotation);
}

inline void Renderer::SetCameraPosition(glm::vec2 position)
{
	camera.position = position;
}

inline void Renderer::SetCameraSize(float size)
{
	camera.size = size;
}

void Renderer::UpdateCamera()
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);

	camera.view = glm::rotate(glm::identity<glm::mat4>(), camera.rotation, glm::vec3(0, 0, 1));
	camera.view = glm::translate(camera.view, glm::vec3(camera.position, -1.0f));
	camera.aspect = (float)w / h;
	camera.projection = glm::ortho<float>(camera.aspect  * -camera.size, camera.aspect * camera.size, -camera.size, camera.size, 0, 100);
	glUniformMatrix4fv(viewProjectionLocation, 1, GL_FALSE, (GLfloat*)&((camera.projection * camera.view)[0]));
}
