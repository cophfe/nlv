#pragma once
#include "GameSystem.h"
#include "Texture.h"

class BalancerSystem : public GameSystem
{
public:
	static constexpr int INPUT_COUNT = 6;
	static constexpr int DEFAULT_NODE_COUNT = 1;
	static constexpr int OUTPUT_COUNT = 1;

	static constexpr float MOVEMENT_SPEED = 10;
	static constexpr float GRAVITY = -9.81f;
	static constexpr float POLE_MAX_HEIGHT = 0.1f;
	static constexpr float BAR_MIN_HEIGHT = 0.2f;
	static constexpr float SPACE_MAX_HEIGHT = 1.0f;
	static constexpr float SPACE_MIN_HEIGHT = 0.5f;
	static constexpr float POLE_2_LENGTH = 0.7f;
	static constexpr float TRACK_LIMIT = 2.4f;
	static constexpr float POLE_FAILURE_ANGLE = 0.209f; // degrees: 12.0f;
	static constexpr float TIME_STEP = 1.0f / 50.0f;

	struct BalancerDataPack : public GameSystem::DataPack
	{
		float poleAngle;
		float poleVelocity;
		float poleAcceleration;
		float pole2Angle;
		float pole2Velocity;
		float pole2Acceleration;
		float cartPosition;
		float cartVelocity;
		float cartAcceleration;

		virtual ~BalancerDataPack() = default;

	};

	BalancerSystem();
	virtual ~BalancerSystem() override = default;
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

private:
	BalancerDataPack defaultDataPack;
	Texture minecart;
};

