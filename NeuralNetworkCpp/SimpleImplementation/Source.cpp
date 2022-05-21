#include "NetworkEvolver.h"
#include <crtdbg.h>
#include "SnakeGame.h"

void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	/*Network network = Network(10, {30, 40, 50, 20}, 10);
	NetworkEvolverDefinition def(network, 40, 100, 0.2f, 0.03f, StepFunction, EvolverMutationType::Set, EvolverCrossoverType::Uniform, EvolverSelectionType::Ranked,
	 false, nullptr, nullptr, 0);
	NetworkEvolver evolver(def);*/

	SnakeGame game;
	game.Run();
}