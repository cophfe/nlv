#pragma once
#include "Network.h"
class NetworkEvolver;

class NetworkOrganism
{
	friend NetworkEvolver;
private:
	// The brain of the organism (also the genome values, since the genome is directly encoded)
	Network network;
	// The inputs to the organism's brain.
	float* networkInputs;
	// The amount of steps the organism has taken
	unsigned int steps;
public:
	// The current fitness of the organism
	float fitness;
	//whether the organism should continue stepping or not
	bool continueStepping;
	
	inline const float* GetNetworkOutputActivations() { return network.GetPreviousActivations(); }
	inline float* GetNetworkInputArray() { return networkInputs; }
	
};

