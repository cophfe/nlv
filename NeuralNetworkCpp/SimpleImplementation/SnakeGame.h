#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include "NetworkEvolver.h"

constexpr int POPULATION_SIZE = 1000;
constexpr int GRID_SIZE = 100;

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
	struct Coord
	{
		Coord() = default;
		Coord(short x, short y) : x(x), y(y) {}
		short x, y;
	};
	struct SnakeSystem
	{
		std::minstd_rand random;
		Coord appleCoord;
		std::vector<Coord> body;
		Direction movementDirection;
	};

	static void OnStartGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms);
	static void StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex);
	static void SetNetworkInputs(SnakeSystem& system, float* inputs);
	static Direction GetLocalDirection(Direction direction, Direction relativeDirection);
	static Coord AddDirectionToCoord(Coord coord, Direction direction);

	std::uniform_int_distribution<short> dist = std::uniform_int_distribution<short>(0, 100);
	SnakeSystem systems[POPULATION_SIZE];
};

