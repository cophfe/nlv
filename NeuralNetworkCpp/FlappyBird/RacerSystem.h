#pragma once
#include "GameSystem.h"
#include "glm.hpp"

class RacerSystem : public GameSystem
{
	//Inputs are:
	// raycast values for forward, left-forward, right-forward, right, left
	// current speed + rotational speed
	//Outputs are:
	//acceleration force, rotational acceleration force
	static constexpr int INPUT_COUNT = 7;
	static constexpr int DEFAULT_NODE_COUNT = 10;
	static constexpr int OUTPUT_COUNT = 2;
	
	static constexpr float MAX_STOP_TIME = 3.0f;
	static constexpr float FORCE = 5.0f;
	static constexpr float ROTATE_FORCE = 4.0f;
	static constexpr float RAYCAST_DISTANCE = 2.0f;
	static constexpr glm::vec2 CAR_DIMENSIONS = { 0.5f, 0.25f };
	
	struct RacerDataPack : public GameSystem::DataPack
	{
		glm::vec2 position;
		float rotation;
		glm::vec2 velocity;
		float rotationalVelocity;
		//(how long it has been stopped for)
		float stoppedTime;
		virtual ~RacerDataPack() = default;
	};

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
	virtual int GetInputCount() const override { return INPUT_COUNT; }
	virtual int GetOutputCount() const override { return OUTPUT_COUNT; }
	virtual int GetDefaultHiddenNodes() const override { return DEFAULT_NODE_COUNT; }
};

