#pragma once
#include "Graphics.h"
#include <vector>
#include "NetworkEvolver.h"
#include "Renderer.h"

constexpr float TIME_STEP = 1.0f / 50.0f;

class GameSystem
{
public:
	GameSystem() = default;
	virtual ~GameSystem() = default;
	GameSystem& operator= (const GameSystem& other) = delete;
	GameSystem(const GameSystem& other) = delete;
	
	//contains information specific to individual organisms.
	//e.g for pole balancer it will hold velocity, position, pole angle
	struct DataPack {

		virtual ~DataPack() = default;
	};

	//set the default datapack to values on the start of a generation
	virtual void SetDefaultDataPack(std::minstd_rand& random) = 0;
	//the logic for stepping through the organism throughout an episode
	virtual void StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping) = 0;
	//the logic for setting the neural network's inputs
	virtual void SetNetworkInputs(DataPack* data, float* networkInputArray) = 0;
	//called on the start of an organism preview (duh)
	virtual void StartOrganismPreview(bool manual, Renderer& renderer) {};
	//the logic for clearing the manual output after a manual call for step organism
	virtual void ResetManualOutput() = 0;
	//for setting manual control (only called when manual control is used)
	virtual void OnKeyPressed(Renderer& renderer, int keycode, int action, bool manual) = 0;
	virtual void OnMousePressed(Renderer& renderer, int button, int action, bool manual) {};
	virtual void OnMouseScrolled(Renderer& renderer, float amount, bool manual) {};
	//called in the middle of drawing
	virtual void DrawGame(DataPack* data, Renderer& renderer) = 0;
	//called when starting and ending rungeneration();
	virtual void OnStartEndGeneration(bool start) {};

	inline std::vector<float>& GetManualOutput() { return manualOutput; }
	virtual DataPack* GetDefaultDataPack() = 0;
	virtual DataPack* NewDataPack() const = 0;
	virtual void CopyDataPack(DataPack* dest, DataPack* src) const = 0;
	virtual int GetInputCount() const  = 0;
	virtual int GetOutputCount() const = 0;
	virtual int GetDefaultHiddenNodes() const = 0;
	virtual float GetStepSpeedMultiplier() const { return 1.0f;  }
protected:
	//the 'output' of the neural network, except controlled by the player.
	//for flappy bird, manualOutput[0] would be set to one when pressing space, for example
	std::vector<float> manualOutput;
};

