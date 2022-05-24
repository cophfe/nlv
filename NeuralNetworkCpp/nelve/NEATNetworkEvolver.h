#pragma once
#include <cstdint>
#include "Maths.h"
#include <vector>

class NetworkEvolverNEAT
{
public:
	NetworkEvolverNEAT(uint32_t inputCount, uint32_t outputCount);

private:

	//NetworkNEAT is the genome
	struct NetworkNEAT
	{
		enum class NodeType 
		{
			SENSOR,
			OUTPUT,
			HIDDEN
		};

		struct Node 
		{
			float data;
			NodeType type;
		};

		struct Connection
		{
			//weight of connection
			float weight;
			//in node
			unsigned short in;
			//out node
			unsigned short out;
			//innovation number
			unsigned short innovation;
			//whether enabled or disables
			bool enabled;
		};

		std::vector<Node> sensorNodes;
		std::vector<Node> outputNodes;
		std::vector<Node> hiddenNodes;
		std::vector<Connection> connections;

		void Mutate();
	};
};

