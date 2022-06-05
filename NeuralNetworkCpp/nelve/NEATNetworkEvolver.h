#pragma once
#include <cstdint>
#include "Maths.h"
#include <vector>

class NetworkEvolverNEAT
{
public:
	NetworkEvolverNEAT(uint32_t inputCount, uint32_t outputCount);

private:
	struct Node
	{
		float data;
	};

	struct Connection
	{
	public:
		Connection(float weight, short in, short out, short innovation, bool enabled) 
		 : weight(weight), in(in), out(out), innovation(innovation), enabled(enabled)
		{}

		//weight of connection
		float weight;
		//in node
		unsigned short in;
		//out node
		unsigned short out;
		//innovation number
		unsigned short innovation;
		//whether enabled or disabled
		bool enabled;
	};

	//NetworkNEAT is the genome
	struct NEATGenome
	{
		std::vector<Node> inputNodes, nodes, outputNodes;
		std::vector<Connection> connections;

		void Mutate();
	};

	struct NEAT
};

