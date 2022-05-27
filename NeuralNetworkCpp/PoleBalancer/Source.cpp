#include "NetworkEvolver.h"
#include <crtdbg.h>
#include "PoleGame.h"

void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Application game;
	game.Run();
}