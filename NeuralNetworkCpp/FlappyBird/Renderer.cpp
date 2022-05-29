#include "Renderer.h"

void Renderer::SetupWindow(GLuint width, GLuint height, const char* title)
{
	if (glfwInit() == GL_FALSE)
		throw std::runtime_error("Something went wrong!");

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		throw std::runtime_error("Something went wrong!");
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGL())
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		throw std::runtime_error("Something went wrong!");
	}

	glfwSwapInterval(0);
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//now setup shader
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
    Colour = vec4(_Colour, 1.0) * texture(_Texture, TexCoord);
})";
	glShaderSource(fragment, 1, &fragmentString, nullptr);
	glCompileShader(fragment);

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
	gl_Position = _ViewProjection * (_Model * vec4(position, 1.0, 1.0));
})";
	glShaderSource(vertex, 1, &vertexString, nullptr);
	glCompileShader(vertex);
	shaderID = glCreateProgram();
	glAttachShader(shaderID, vertex);
	glAttachShader(shaderID, fragment);
	glLinkProgram(shaderID);
	//and now quad
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
	int indices[6] = {
		0, 1, 2,
		0, 2, 3
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(int), (void*)indices, GL_STATIC_DRAW);
	//define position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	defaultTexture.Load("white.png");
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
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
	glUniform3f(glGetUniformLocation(shaderID, "_Colour"), colour.x, colour.y, colour.z);
	//calculate matrix
	
}

void Renderer::DrawBox(glm::vec2 position, float width, float height, float rotation, glm::vec3 colour, glm::vec2 pivot)
{
	DrawSprite(&defaultTexture, position, width, height, rotation, colour, pivot);
}

inline void Renderer::SetCameraRotation(float rotation)
{
	camera.
}

inline void Renderer::SetCameraPosition(glm::vec2 position)
{
}

inline void Renderer::SetCameraScale(float scale)
{
}
