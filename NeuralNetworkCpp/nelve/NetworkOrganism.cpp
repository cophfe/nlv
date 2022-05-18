#include "NetworkOrganism.h"

NetworkOrganism::NetworkOrganism(Network& networkToCopy)
	: network(Network(networkToCopy)), fitness(0), continueStepping(true), steps(0), networkInputs(new float[network.GetInputCount()])
{
}

NetworkOrganism::~NetworkOrganism()
{
	if (networkInputs != nullptr)
	{
		delete[] networkInputs;
		networkInputs = nullptr;
	}
}

NetworkOrganism::NetworkOrganism(const NetworkOrganism& other)
	: network(other.network), steps(other.steps), fitness(other.fitness),
	continueStepping(other.continueStepping)
{
	networkInputs = new float[other.network.GetInputCount()];
	memcpy(other.networkInputs, networkInputs, sizeof(float) * other.network.GetInputCount());
}

NetworkOrganism& NetworkOrganism::operator=(const NetworkOrganism& other)
{
	if (networkInputs != nullptr)
	{
		delete[] networkInputs;
		networkInputs = nullptr;
	}

	network = other.network;
	steps = other.steps;
	fitness = other.fitness;
	continueStepping = other.continueStepping;
	networkInputs = new float[other.network.GetInputCount()];
	memcpy(other.networkInputs, networkInputs, sizeof(float) * other.network.GetInputCount());
	return *this;
}

void NetworkOrganism::Reset()
{
	fitness = 0;
	continueStepping = true;
	steps = 0;
}
