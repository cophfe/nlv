#include "NetworkEvolver.h"
#include <crtdbg.h>
#include "FlappyBird.h"

void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	FlappyBird game;
	game.Run();
}