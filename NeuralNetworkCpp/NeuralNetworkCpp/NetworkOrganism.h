#pragma once
#include "Network.h"
class NetworkEvolver;

class NetworkOrganism
{
	friend NetworkEvolver;
private:
	inline NetworkOrganism(Network& networkToCopy) : network(Network(networkToCopy)), fitness(0), continueStepping(true), steps(0) { networkInputs = new float[network.GetInputCount() ]; };
	~NetworkOrganism();
	NetworkOrganism(const NetworkOrganism& other);
	NetworkOrganism& operator=(const NetworkOrganism& other);
	
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
	
	/// <returns>The activation array from the outputs of the organism's neural network</returns>
	inline const float* GetNetworkOutputActivations() const { return network.GetPreviousActivations(); }
	/// <returns>The array containing the input values to the organism's neural network</returns>
	inline float* GetNetworkInputArray() { return networkInputs; }
	
};

