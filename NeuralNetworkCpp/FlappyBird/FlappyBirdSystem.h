#pragma once
#include "GameSystem.h"
#include "glm.hpp"

class FlappyBirdSystem : public GameSystem
{
public:
	static constexpr float BAR_GAIN = 1.0f;
	static constexpr float EXIST_GAIN = 0.2f;
	static constexpr float FLAP_LOSS = 0.05f;

	static constexpr float MOVEMENT_SPEED = 2;
	static constexpr float GRAVITY = -9.81f;
	static constexpr float SCREEN_HALF_HEIGHT = 3.0f;
	static constexpr float SCREEN_HALF_WIDTH = 5.0f;
	static constexpr float BAR_MIN_HEIGHT = 0.3f;
	static constexpr float SPACE_MAX_HEIGHT = 3.0f;
	static constexpr float SPACE_MIN_HEIGHT = 0.75f;
	static constexpr float BAR_MAX_HEIGHT = SCREEN_HALF_HEIGHT * 2 - 2.0f;
	static constexpr float BAR_HALF_WIDTH = 0.3f;
	static constexpr float BAR_DISTANCE = 4.0f;
	static constexpr float JUMP_FORCE = 3.24f;
	static constexpr float BIRD_RADIUS = 0.25f;

	static constexpr int INPUT_NODES = 5;
	static constexpr int DEFAULT_HIDDEN_NODES = 6;
	static constexpr int OUTPUT_NODES = 1;
	FlappyBirdSystem();
	virtual ~FlappyBirdSystem();

	struct FlappyBirdDataPack : public DataPack 
	{
		float yVelocity;
		float yPos;
		float barXPos;
		float barHeight;
		float spaceHeight;
		std::minstd_rand random;
	};

	// Inherited via GameSystem
	virtual void SetDefaultDataPack(std::minstd_rand& random) override;

	virtual void StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping) override;

	virtual void SetNetworkInputs(DataPack* data, float* networkInputArray) override;

	virtual void ResetManualOutput() override;

	virtual void OnKeyPressed(GLFWwindow* window, int keycode, int action) override;

	virtual void DrawGame(DataPack* data, Renderer& renderer) override;

	virtual DataPack* NewDataPack() const override;
	virtual void CopyDataPack(DataPack* dest, DataPack* src) const override;

	virtual int GetInputCount() const override { return INPUT_NODES; }
	virtual int GetOutputCount() const override { return OUTPUT_NODES; }
	virtual int GetDefaultHiddenNodes() const override { return DEFAULT_HIDDEN_NODES; }
	virtual DataPack* GetDefaultDataPack() override { return &defaultDataPack; }
private:
	FlappyBirdDataPack defaultDataPack;

	Texture bird;
};

