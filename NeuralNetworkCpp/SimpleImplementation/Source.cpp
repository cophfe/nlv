#include "Network.h"

void main()
{
	Network network = Network(10, {30, 40, 50, 20}, 10);
	network.RandomizeValues();

	const float* output = network.Evaluate(nullptr, 0);

	
}