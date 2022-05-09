#pragma once
#include "Network.h"

class NetworkEvolver
{
public:
	
	NetworkEvolver();

	//can input a network for initial state of the simulator
	NetworkEvolver(Network& network);
	
	void RunGeneration(int generationCount);
};

