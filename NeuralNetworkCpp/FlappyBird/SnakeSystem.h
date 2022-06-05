#pragma once
#include "GameSystem.h"


class SnakeSystem : public GameSystem
{
public:
	static constexpr int INPUT_NODES = 4;
	static constexpr int DEFAULT_HIDDEN_NODES = 10;
	static constexpr int OUTPUT_NODES = 3;
	
	static constexpr int APPLE_GAIN = 75;
	static constexpr int GRID_SIZE = 30;
	static constexpr int SIZE_GAIN = 4;
	static constexpr int MAX_REMAINING_STEPS = 300;
	static constexpr int STARTING_STEPS = 200;

	struct Coord
	{
		Coord() = default;
		Coord(short x, short y) : x(x), y(y) {}
		Coord&& operator* (const int val) {
			return { (short)(x * val), (short)(y * val) };
		}
		Coord&& operator/ (const int val) {
			return { (short)(x / val), (short)(y / val) };
		}
		Coord&& operator* (const float val) {
			return { (short)(x * val), (short)(y * val) };
		}
		Coord&& operator/ (const float val) {
			return { (short)(x / val), (short)(y / val) };
		}
		Coord&& operator+ (const Coord& other) {
			return { x + other.x, y + other.y };
		}
		Coord&& operator- (const Coord& other) {
			return { x - other.x, y - other.y };

		}
		bool operator== (const Coord& other) {
			return x == other.x && y == other.y;
		}
		short x, y;
	};
	enum class Direction : char
	{
		LEFT,
		UP,
		RIGHT,
		DOWN,
		COUNT
	};
	struct SnakeDataPack : public DataPack
	{
		std::minstd_rand random;
		Coord appleCoord;
		std::vector<Coord> body;
		Direction movementDirection;
		int stepsLeft;
		int amountToAdd;

		virtual ~SnakeDataPack() = default;
	};

	SnakeSystem();
	virtual ~SnakeSystem() = default;

	// Inherited via GameSystem
	virtual void SetDefaultDataPack(std::minstd_rand& random) override;
	virtual void StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping) override;
	virtual void SetNetworkInputs(DataPack* data, float* networkInputArray) override;
	virtual void ResetManualOutput() override;
	virtual void OnKeyPressed(GLFWwindow* window, int keycode, int action) override;
	virtual void DrawGame(DataPack* data, Renderer& renderer) override;
	virtual DataPack* GetDefaultDataPack() override;
	virtual DataPack* NewDataPack() const override;
	virtual void CopyDataPack(DataPack* dest, DataPack* src) const override;
	virtual int GetInputCount() const override { return INPUT_NODES; }
	virtual int GetOutputCount() const override { return OUTPUT_NODES; }
	virtual int GetDefaultHiddenNodes() const override { return DEFAULT_HIDDEN_NODES; }
	virtual float GetStepSpeedMultiplier() const override { return 0.14f; }
private:

	enum class SystemState : char
	{
		GAME_OVER,
		APPLE_GET,
		NOTHING,
	};
	SystemState StepSystem(SnakeDataPack& system);
	static Direction GetLocalDirection(Direction direction, Direction relativeDirection);
	static Coord AddDirectionToCoord(Coord coord, Direction direction);

	std::minstd_rand random;
	std::uniform_int_distribution<short> dist = std::uniform_int_distribution<short>(0, GRID_SIZE - 1);
	SnakeDataPack defaultDataPack;
};

