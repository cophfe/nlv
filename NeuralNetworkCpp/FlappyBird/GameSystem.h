#pragma once
#include "Graphics.h"
#include <vector>
#include "NetworkEvolver.h"

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

	};

	//set the default system to values on the start of a generation
	virtual void SetDefaultSystem(std::minstd_rand& random) = 0;
	//the logic for stepping through the organism throughout an episode
	virtual void StepOrganism(DataPack* data, const float* networkOutput, float& fitness, bool& continueStepping) = 0;
	//the logic for setting the neural network's inputs
	virtual void SetNetworkInputs(DataPack* data, float* networkInputArray) = 0;
	//the logic for clearing the manual output after a manual call for step organism
	virtual void ResetManualOutput() = 0;
	//for setting manual control (only called when manual control is used)
	virtual void OnKeyPressed(GLFWwindow* window, int keycode, int action) = 0;
	//called in the middle of drawing
	virtual void DrawGame(DataPack* data) = 0;

	inline std::vector<float>& GetManualOutput() { return manualOutput; }
	virtual DataPack* GetDefaultSystem() = 0;
	virtual int GetInputCount() = 0;
	virtual int GetOutputCount() = 0;
	virtual int GetDefaultHiddenNodes() = 0;
protected:
	//the 'output' of the neural network, except controlled by the player.
	//for flappy bird, manualOutput[0] would be set to one when pressing space, for example
	std::vector<float> manualOutput;
};

