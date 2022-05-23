#pragma once
class NEATNetworkEvolver
{






	struct Genome
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
			unsigned short in;
			unsigned short out;
			float weight;
			bool enabled;
			unsigned short innov;
		};

		
	};
};

