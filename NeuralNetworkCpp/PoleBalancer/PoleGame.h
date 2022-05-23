#pragma once
#include "glm.hpp"
#include "NetworkEvolver.h"
#include "raylib.h"
#include <vector>
#include <string>
#include <random>
#include <format>
#include <iostream>
#include <algorithm>
#include "NetworkEvolver.h"
#include "Button.h"

//basic test of genetic algorithm
//https://researchbank.swinburne.edu.au/file/62a8df69-4a2c-407f-8040-5ac533fc2787/1/PDF%20(12%20pages).pdf

typedef glm::vec2 Vec2;
constexpr int POPULATION_SIZE = 1000;
constexpr float FORCE = 10;
constexpr float GRAVITY = -9.81f;
constexpr float POLE_MASS = 0.1f;
constexpr float CART_MASS = 1.0f;
constexpr float POLE_LENGTH = 0.5f;
constexpr float TRACK_LIMIT = 2.4f;
constexpr float POLE_FAILURE_ANGLE = 0.209f; // degrees: 12.0f;
constexpr float TIME_STEP = 1.0f / 50.0f;
constexpr float MAX_TIME = 60.0f;
constexpr int MAX_STEPS = MAX_TIME/TIME_STEP;

struct PoleSystem
{
	float poleAngle;
	float poleVelocity;
	float poleAcceleration;
	float cartPosition;
	float cartVelocity;
	float cartAcceleration;
};

class PoleGame
{
public:
	void Run();
	PoleGame();
	~PoleGame();
private:
	static void OnStartGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms);
	static void StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex);
	static void SetNetworkInputs(PoleSystem& system, float* inputs);
	void StepOrganism(PoleSystem& system, float networkOutput, float& fitness, bool& continueStepping);
	void SetupStartSystem();
	void DrawGame(PoleSystem& system, short x, short y, short sizeX, short sizeY, short floorSize);

	PoleSystem templateSystem;
	std::minstd_rand random;
	std::normal_distribution<float> dist = std::normal_distribution<float>(0, 0.03f);
	std::uniform_int_distribution<short> indexDist = std::uniform_int_distribution<short>(0, POPULATION_SIZE - 1);
	PoleSystem systems[POPULATION_SIZE];
	float deltaTime;		
	std::chrono::high_resolution_clock::time_point lastTime;
};


