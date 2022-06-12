#pragma once
#include "Network.h"

namespace nlv 
{
	class NetworkEvolver;

	class NetworkOrganism
	{
		friend NetworkEvolver;
	private:
		NetworkOrganism(Network& networkToCopy);
		~NetworkOrganism();
		NetworkOrganism(const NetworkOrganism& other);
		NetworkOrganism& operator=(const NetworkOrganism& other);

		// The brain of the organism (also the genome values, since the genome is directly encoded)
		Network network;
		// The inputs to the organism's brain.
		float* networkInputs;
		// The amount of steps the organism has taken
		uint32_t steps;

		//Reset values used in reinforcement learning episode
		void Reset();
	public:
		// The current fitness of the organism
		float fitness;
		//whether the organism should continue stepping or not
		bool continueStepping;

		// Returns the activation array from the outputs of the organism's neural network (all values are between 0 and 1)
		inline const float* GetNetworkOutputActivations() const { return network.GetPreviousActivations(); }
		// Returns the array containing the input values to the organism's neural network
		inline float* GetNetworkInputArray() { return networkInputs; }
		
		const Network& GetNetwork() const { return network; }
		float GetFitness() const { return fitness; }
		uint32_t GetStepsTaken() const { return steps; }
	};
}
