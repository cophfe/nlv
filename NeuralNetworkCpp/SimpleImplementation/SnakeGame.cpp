#include "SnakeGame.h"

SnakeGame::SnakeGame()
{
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
	InitWindow(1000, 600, "Snake Test");
	

}

void SnakeGame::Run()
{
	Network network = Network(4, {6}, 3);
	NetworkEvolverDefinition def(network, POPULATION_SIZE, 1000, 0.2f, 0.03f, StepFunction, EvolverMutationType::Set, EvolverCrossoverType::Uniform, EvolverSelectionType::FitnessProportional,
	 false, OnStartGeneration, nullptr, time(0));
	NetworkEvolver evolver(def);
	evolver.SetUserPointer(this);
	srand(time(0)); //for seeds for each generation

	while (!WindowShouldClose())
	{

		BeginDrawing();
		ClearBackground(WHITE);
		auto mousePos = GetMousePosition();

		bool overTestGeneration = mousePos.x > 100 && mousePos.x < 300 && mousePos.y > 150 && mousePos.y < 200;
		if (overTestGeneration)
			DrawRectangle(100, 150, 200, 50, MAROON);
		else
			DrawRectangle(100, 150, 200, 50, RED);
		DrawText("Do Generation", 110, 160, 21, BLACK);
		DrawFPS(4, 4);

		std::string txt = "Generation: " + std::to_string(evolver.GetGeneration());
		DrawText(txt.c_str(), 50, 40, 50, BLACK);
		std::string txt2 = "Best Fitness: " + std::to_string(evolver.FindBestOrganism().fitness);
		DrawText(txt2.c_str(), 50, 80, 50, BLACK);

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && overTestGeneration)
		{
			
			DrawRectangle(0, 0, 1000, 600, { 0,0,0, 80 });
			DrawText("...", 100, 275, 50, BLACK);
			EndDrawing();
			evolver.EvaluateGeneration();
		}
		else
			EndDrawing();

	}
}

SnakeGame::~SnakeGame()
{
	CloseWindow();
}

void SnakeGame::OnStartGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms)
{
	SnakeGame* ptr = (SnakeGame*)evolver.GetUserPointer();
	
	//this needs to be the only use of rand(), for determinism
	int seed = rand();

	//this is kinda bad since all snakes should have the exact same values, so should only calculate once per generation, not once per snake per generation
	for (size_t i = 0; i < POPULATION_SIZE; i++)
	{
		//setup initial values for every snake system (all the same)
		auto& sys = ptr->systems[i];
		sys.body.clear();
		sys.random.seed(seed);
		sys.appleCoord = Coord{ ptr->dist(sys.random), ptr->dist(sys.random) };
		sys.movementDirection = Direction::UP;
		sys.body.emplace_back(GRID_SIZE / 2, GRID_SIZE / 2);
		//set network inputs
		SetNetworkInputs(sys, organisms[i].GetNetworkInputArray());
	}
}

void SnakeGame::StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex)
{
	//read output actions
	const float* activations = organism.GetNetworkOutputActivations();
	char maxIndex = 0;
	if (activations[1] > activations[0])
		maxIndex = 1;
	if (activations[2] > activations[maxIndex])
		maxIndex = 2;
	auto& sys = ((SnakeGame*)evolver.GetUserPointer())->systems[organismIndex];

	char moveDir = (char)sys.movementDirection;
	switch (maxIndex)
	{
	case 0: //Turn left
		moveDir--;
		break;
	case 2: //Turn right
		moveDir++;
		break;
	//case 1 is move forward
	}
	if (moveDir < 0)
		moveDir = (char)Direction::COUNT - 1;
	if (moveDir >= (char)Direction::COUNT)
		moveDir = 0;
	sys.movementDirection = SnakeGame::Direction(moveDir);

	//actually move
	Coord newCoord = AddDirectionToCoord(sys.body[0], sys.movementDirection);

	//if head intersects with body pos, game over
	for (Coord pos : sys.body)
	{
		if (pos.x == newCoord.x && pos.y == newCoord.y)
		{
			organism.continueStepping = false;
			return;
		}
	}
	//if head outside of bounds, game over
	if (newCoord.x >= GRID_SIZE || newCoord.x < 0 || newCoord.y >= GRID_SIZE || newCoord.y < 0)
	{
		organism.continueStepping = false;
		return;
	}

	//if no die add head to body
	sys.body.insert(sys.body.begin(), newCoord);
	//if head intersects with apple, be happy
	if (newCoord.x == sys.appleCoord.x && newCoord.y == sys.appleCoord.y)
	{
		organism.fitness += 10.0f;
	}
	//otherwise delete last snake bodypart
	else
		sys.body.pop_back();

	//just for existing, add some fitness
	organism.fitness += 0.1f;

	//finally, set inputs for next step
	SetNetworkInputs(sys, organism.GetNetworkInputArray());
}

void SnakeGame::SetNetworkInputs(SnakeSystem& system, float* inputs)
{
	Coord head = system.body[0];
	auto left = AddDirectionToCoord(head, GetLocalDirection(system.movementDirection, Direction::LEFT));
	auto right = AddDirectionToCoord(head, GetLocalDirection(system.movementDirection, Direction::RIGHT));
	auto forward = AddDirectionToCoord(head, system.movementDirection);
	
	//if wall is in any of the directions
	bool isOnLeft = left.x < 0 || left.x >= GRID_SIZE || left.y < 0 || left.y >= GRID_SIZE;
	bool isOnRight = right.x < 0 || right.x >= GRID_SIZE || right.y < 0 || right.y >= GRID_SIZE;
	bool isInFront = forward.x < 0 || forward.x >= GRID_SIZE || forward.y < 0 || forward.y >= GRID_SIZE;
	//if body is in any of the directions
	for (Coord bodyPart : system.body)
	{
		isOnLeft |= left.x == bodyPart.x && left.y == bodyPart.y;
		isOnRight |= right.x == bodyPart.x && right.y == bodyPart.y;
		isInFront |= forward.x == bodyPart.x && forward.y == bodyPart.y;
	}

	//if there is a bodypart or wall on left side of head
	inputs[0] = (float)isOnLeft;
	//if there is a bodypart or wall on right side of head
	inputs[1] = (float)isOnRight;
	//if there is a bodypart or wall in front of head
	inputs[2] = (float)isOnLeft;
	//angle to apple
	float x = system.appleCoord.x - head.x, y = system.appleCoord.y - head.y;
	float invSq = 1.0f / sqrtf(x * x + y * y);
	x *= invSq;
	y *= invSq;
	inputs[3] = sinf(x);
}

SnakeGame::Direction SnakeGame::GetLocalDirection(Direction direction, Direction relativeDirection)
{
	char dir = (char)direction;
	switch (relativeDirection)
	{
	case SnakeGame::Direction::LEFT:
		dir--;
		break;
	case SnakeGame::Direction::RIGHT:
		dir++;
		break;
	case SnakeGame::Direction::DOWN:
		dir += 2;
		break;
	}
	dir %= (char)Direction::COUNT;
	if (dir < 0)
		dir += (char)Direction::COUNT;

	return (Direction)dir;
}

SnakeGame::Coord SnakeGame::AddDirectionToCoord(Coord coord, Direction direction)
{
	switch (direction)
	{
	case SnakeGame::Direction::LEFT:
		return { coord.x - 1, coord.y };
	case SnakeGame::Direction::UP:
		return { coord.x, coord.y + 1};
	case SnakeGame::Direction::RIGHT:
		return { coord.x + 1, coord.y};
	case SnakeGame::Direction::DOWN:
		return { coord.x, coord.y - 1};
	}
	return coord;
}
