#pragma once
#include "GameSystem.h"
#include "glm.hpp"
#include "Spline.h"

class RacerSystem : public GameSystem
{
public:
	static constexpr int INPUT_COUNT = 7;
	static constexpr int DEFAULT_NODE_COUNT = 10;
	static constexpr int OUTPUT_COUNT = 2;
	static constexpr float TIME_STEP = 1.0f / 50.0f;

	static constexpr float MAX_STOP_TIME = 1.0f;

	static constexpr float MAX_SPEED = 30.0f;
	static constexpr float ACCELERATION = 18.0f;
	static constexpr float DRIFT_CANCEL = 10.0f;
	static constexpr float TURN_SPEED = 2.8f;
	static constexpr float DRAG = 3.0f;
	static constexpr float DEAD_ZONE = 0.2f;

	static constexpr float RAYCAST_DISTANCE = 13.0f;
	static constexpr glm::vec2 CAR_DIMENSIONS = { 6, 3 };
	
	struct RacerDataPack : public GameSystem::DataPack
	{
		glm::vec2 position;
		float rotation;
		glm::vec2 velocity;
		//(how long it has been stopped for)
		float stoppedTime;
		virtual ~RacerDataPack() = default;
	};

	RacerSystem();
	virtual ~RacerSystem() = default;

	// Inherited via GameSystem
	virtual void SetDefaultDataPack(std::minstd_rand& random) override;
	virtual void StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping) override;
	virtual void SetNetworkInputs(DataPack* data, float* networkInputArray) override;
	virtual void StartOrganismPreview(bool manual, Renderer& renderer) override;
	virtual void OnStartEndGeneration(bool start) override;
	virtual void ResetManualOutput() override;
	virtual void OnKeyPressed(Renderer& renderer, int keycode, int action, bool manual) override;
	virtual void OnMousePressed(Renderer& renderer, int button, int action, bool manual) override;
	virtual void OnMouseScrolled(Renderer& renderer, float amount, bool manual) override;
	virtual void DrawGame(DataPack* data, Renderer& renderer) override;
	virtual DataPack* GetDefaultDataPack() override;
	virtual DataPack* NewDataPack() const override;
	virtual void CopyDataPack(DataPack* dest, DataPack* src) const override;
	virtual int GetInputCount() const override { return INPUT_COUNT; }
	virtual int GetOutputCount() const override { return OUTPUT_COUNT; }
	virtual int GetDefaultHiddenNodes() const override { return DEFAULT_NODE_COUNT; }

private:
	RacerDataPack defaultDataPack;

	//used raycast
	bool RaycastLine(std::vector<glm::vec2>& line, glm::vec2 origin, glm::vec2 dir, float raycastDistance, float* hitDistance);
	bool RaycastWalls(glm::vec2 origin, glm::vec2 dir, float raycastDistance, float* hitDistance);

	Texture carTexture;
	bool canEdit = true;
	bool editing = false;
	bool showDebug = false;
	bool followCar = false;

	bool holdingPoint = false;
	int heldControlPointIndex = -1;
	float radius = 8.0f;

	void BuildTrack();
	Spline raceSpline;
	std::vector<glm::vec2> leftArray;
	std::vector<glm::vec2> rightArray;
	std::vector<glm::vec2> raceArray;

	glm::vec2 lastMousePos;
	bool dragging;
};

