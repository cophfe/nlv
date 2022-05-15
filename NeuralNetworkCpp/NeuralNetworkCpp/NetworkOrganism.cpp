#include "NetworkOrganism.h"

NetworkOrganism::~NetworkOrganism()
{
	if (networkInputs != nullptr)
	{
		delete[] networkInputs;
		networkInputs = nullptr;
	}
}

NetworkOrganism::NetworkOrganism(const NetworkOrganism& other)
{
	network = other.network;
	steps = other.steps;
	fitness = other.fitness;
	continueStepping = other.continueStepping;
	networkInputs = new float[other.network.GetInputCount()];
}

NetworkOrganism& NetworkOrganism::operator=(const NetworkOrganism& other)
{
	return *this;
}
