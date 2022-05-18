#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <random>
#include <format>
#include <iostream>
#include <algorithm>
#include "NetworkEvolver.h"
#include "Button.h"

constexpr int POPULATION_SIZE = 2000;
constexpr int MAX_STEPS = 10000;
constexpr int STARTING_STEPS = 200;
constexpr float FITNESS_GAIN = 0.5f;
//too high, and it becomes locally beneficial to just loop around in a way that cannot harm you but also means you cannot get more apples
//too low and it is too hard for them :(
constexpr int APPLE_GAIN = 75;
constexpr int GRID_SIZE = 30;
constexpr float STEP_TIME = 0.1f;

//this snake implementation is based on this resource: http://ceur-ws.org/Vol-2468/p9.pdf
class SnakeGame
{
public:
	SnakeGame();
	void Run();
	~SnakeGame();
private:
	enum class Direction : char
	{
		LEFT,
		UP,
		RIGHT,
		DOWN,
		COUNT
	};
	enum class SystemState : char
	{
		GAME_OVER,
		APPLE_GET,
		NOTHING,
	};
	
	struct SnakeSystem
	{
		std::minstd_rand random;
		Coord appleCoord;
		std::vector<Coord> body;
		Direction movementDirection;
		int stepsLeft;
	};
	

	static void OnStartGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms);
	static void StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex);
	static void SetNetworkInputs(SnakeSystem& system, float* inputs);
	void StepOrganism(SnakeSystem& system, const float* networkOutputs, float& fitness, bool& continueStepping);
	SystemState StepSystem(SnakeSystem& system);
	static Direction GetLocalDirection(Direction direction, Direction relativeDirection);
	static Coord AddDirectionToCoord(Coord coord, Direction direction);
	void SetupStartSystem();
	void DrawSnakeGame(SnakeSystem& system, Coord pos, short size);

	SnakeSystem startSystem;
	std::minstd_rand random;
	std::uniform_int_distribution<short> dist = std::uniform_int_distribution<short>(0, GRID_SIZE - 1);
	SnakeSystem systems[POPULATION_SIZE];
	float deltaTime;
};

