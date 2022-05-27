#pragma once
#include "GameSystem.h"

class FlappyBirdSystem : public GameSystem
{
public:
	static constexpr float MOVEMENT_SPEED = 10;
	static constexpr float GRAVITY = -9.81f;
	static constexpr float BAR_MIN_HEIGHT = 0.3f;
	static constexpr float BAR_MAX_HEIGHT = 1.0f;
	static constexpr float SPACE_MAX_HEIGHT = 0.6f;
	static constexpr float SPACE_MIN_HEIGHT = 0.3f;
	static constexpr float JUMP_FORCE = 5.0f;

	static constexpr int INPUT_NODES = 6;
	static constexpr int DEFAULT_HIDDEN_NODES = 6;
	static constexpr int OUTPUT_NODES = 1;
	FlappyBirdSystem();
	virtual ~FlappyBirdSystem();

	struct FlappyBirdDataPack : public DataPack 
	{

	};

	// Inherited via GameSystem
	virtual void SetDefaultSystem(std::minstd_rand& random) override;

	virtual void StepOrganism(DataPack* data, const float* networkOutput, float& fitness, bool& continueStepping) override;

	virtual void SetNetworkInputs(DataPack* data, float* networkInputArray) override;

	virtual void ResetManualOutput() override;

	virtual void OnKeyPressed(GLFWwindow* window, int keycode, int action) override;

	virtual void DrawGame(DataPack* data) override;

	virtual int GetInputCount() override { return INPUT_NODES; }
	virtual int GetOutputCount() override { return OUTPUT_NODES; }
	virtual int GetDefaultHiddenNodes() override { return DEFAULT_HIDDEN_NODES; }
	virtual DataPack* GetDefaultSystem() override { return &defaultDataPack; }
private:
	FlappyBirdDataPack defaultDataPack;

};
