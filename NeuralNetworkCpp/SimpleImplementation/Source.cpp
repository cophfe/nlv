#include "NetworkEvolver.h"
#include <crtdbg.h>

static void StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex)
{
	//this is threaded (if specified), so calling rand() will cause issues
	//organism.fitness = std::random_device::random_device()();
	
	//threading has to be false for this
	organism.fitness = (float)rand() / RAND_MAX;
}

void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Network network = Network(10, {30, 40, 50, 20}, 10);
	NetworkEvolverDefinition def(network, 40, 100, 0.2f, 0.03f, StepFunction, EvolverMutationType::Randomized, EvolverCrossoverType::Uniform, EvolverSelectionType::Ranked,
	 false, nullptr, nullptr);
	NetworkEvolver evolver(def);

	srand(time(0));
	std::cout << "\nyo\n";
	evolver.EvaluateGeneration();
	std::cout << "\nyo\n";
	evolver.EvaluateGeneration();
	std::cout << "\nyo\n";
}