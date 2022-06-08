#pragma once
#include "glm.hpp"
#include "NetworkEvolver.h"
#include <vector>
#include <string>
#include <random>
#include <format>
#include <iostream>
#include <algorithm>
#include "NetworkEvolver.h"
#include "Renderer.h"
#include <initializer_list>
#include "GameSystem.h"
#include "FlappyBirdSystem.h"
#include "SnakeSystem.h"
#include "BalancerSystem.h"
#include "RacerSystem.h"

//basic test of genetic algorithm
//https://researchbank.swinburne.edu.au/file/62a8df69-4a2c-407f-8040-5ac533fc2787/1/PDF%20(12%20pages).pdf

typedef glm::vec2 Vec2;
constexpr int THREAD_COUNT = 10;

constexpr int DEFAULT_HIDDEN_NODES = 6;
constexpr int DEFAULT_POPULATION = 1000;
constexpr float DEFAULT_MAX_TIME = 60;
constexpr float DEFAULT_ELITE = 0.05f;
constexpr bool DEFAULT_THREADED = true;
constexpr bool DEFAULT_STATIC = false;
constexpr float DEFAULT_MUTATION_RATE = 0.2f;

class Application
{
public:
	void Run();
	Application();
	Application(const Application& other) = delete;
	Application& operator= (const Application& other) = delete;
	~Application();
private:
	void RunGeneration();
	void RunGenerations();
	void DrawEvolverWindow();
	void DrawDataWindow();
	void DrawPlayWindow();
	void DrawGame();
	void GetEvolverValues();
	static void OnStartGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms);
	static void OnEndGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms);
	static void StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex);
	void SetupDefaultSystem();
	void SetCurrentSolution(int organismIndex);
	void SetCurrentSolutionToPlayMode();
	void RunCurrentSolution();
	void ConfigureEvolver();
	void ClearEvolver();

	Renderer renderer;
	NetworkEvolver evolver;
	GameSystem* gameSystem = nullptr;
	std::minstd_rand random;
	GameSystem::DataPack** dataPacks = nullptr;
	float deltaTime;		
	std::chrono::high_resolution_clock::time_point lastTime;

	//state variables
	enum class GameType : int
	{
		FLAPPY_BIRD,
		POLE_BALANCER,
		SNAKE,
		RACER,
		UNSET 
	};
	void SetGame(GameType type);

	GameType gameType = GameType::UNSET;
	bool evolverIsSetup = false;
	int hiddenLayers = 1;
	std::vector<int> nodesPerLayer { 0 };
	int populationSize = DEFAULT_POPULATION;
	float maxTime = DEFAULT_MAX_TIME;
	int maxSteps = DEFAULT_MAX_TIME / TIME_STEP;
	float elitePercent = DEFAULT_ELITE;
	bool multithread = DEFAULT_THREADED;
	bool staticEpisodes = DEFAULT_STATIC;
	float mutationRate = DEFAULT_MUTATION_RATE;
	int mutationType = 0;
	int selectionType = 0;
	int crossoverType = 0;
	float timeToComplete = 0;
	float progress = 0;
	bool evolverIsRunning = false;
	bool runGenerations = false;
	unsigned int seed = 0;

	std::vector<float> averages;
	std::vector<float> minimums;
	std::vector<float> maximums;
	float maxEver = 0;
	float minEver = 0;

	struct {
		int playSpeed = 0;
		bool isAI = false;
		unsigned int orgIndex = 0;
		Network network;
		GameSystem::DataPack* dataPack;
		float fitness = 0;
		float continueTimer = 0;
		unsigned int steps = 0;
		std::vector<float> inputs;
		bool running = false;
	} currentSolution;

	static void OnKeyPressed(GLFWwindow* window, int keycode, int scancode, int action, int mods);
	static void OnMousePressed(GLFWwindow* window, int button, int action, int mods);
	static void OnMouseScrolled(GLFWwindow* window, double xoffset, double yoffset);
};


