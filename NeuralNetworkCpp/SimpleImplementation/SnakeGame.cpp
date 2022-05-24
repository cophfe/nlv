#include "SnakeGame.h"

SnakeGame::SnakeGame()
{
	SetConfigFlags(FLAG_MSAA_4X_HINT);// | FLAG_VSYNC_HINT);
	InitWindow(1000, 600, "Snake Test");
	random.seed(time(0));

}

void SnakeGame::Run()
{
	//best i have had so far is 1 hidden layer with 6 neurons
	Network network = Network(4, { 6 }, 3);
	NetworkEvolverDefinition def(network, POPULATION_SIZE, MAX_STEPS, 0.02f, 0.6f, StepFunction, EvolverMutationType::Set, EvolverCrossoverType::Point, 
		EvolverSelectionType::Ranked, OnStartGeneration, nullptr, true, true, time(0), 1, 15, 10);
	NetworkEvolver evolver(def);
	evolver.SetUserPointer(this);
	SetupStartSystem();
	evolver.EvaluateGeneration();

	Button doGenerationButton(Coord(25, 200), Coord(250, 50), RED, MAROON, WHITE, 30, 10, "Do Generation", nullptr);
	Button doGenerationsButton(Coord(25, 260), Coord(250, 50), RED, MAROON, WHITE, 30, 10, "Do Generations", nullptr);
	Button runBest(Coord(730, 50), Coord(250, 50), RED, MAROON, WHITE, 30, 10, "Run Best", nullptr);
	Button runRand(Coord(730, 120), Coord(250, 50), RED, MAROON, WHITE, 30, 10, "Run Random", nullptr);
	
	struct {
		Network network;
		SnakeSystem system;
		float fitness = 0;
		int steps = 0;
		float stepTimer = 0;
		float inputs[4];
		bool running = false;
	} testOrganism;
	testOrganism.system = startSystem;

	float highestFitness = -10000000000000000000.0f;
	float averageFitness = 0;
	float lowestFitness = 10000000000000000000.0f;
	const NetworkOrganism* organisms = evolver.GetOrganisms();
	for (int i = 0; i < evolver.GetPopulation(); i++)
	{
		averageFitness += organisms[i].fitness;
		lowestFitness = std::min(organisms[i].fitness, lowestFitness);
		highestFitness = std::max(organisms[i].fitness, highestFitness);
	}
	averageFitness /= evolver.GetPopulation();
	bool lappingGenerations = false;
	lastTime = std::chrono::high_resolution_clock::now();

	while (!WindowShouldClose())
	{
		std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
		deltaTime = (float)std::chrono::duration_cast<std::chrono::duration<double>>(start - lastTime).count();
		lastTime = start;

		BeginDrawing();
		ClearBackground(BLACK);
		auto mousePos = GetMousePosition();


		std::string txt = "Generation: " + std::to_string(evolver.GetGeneration());
		DrawText(txt.c_str(), 25, 25, 50, WHITE);
		std::string txt2 = "Best Fitness: " + std::format("{:.2f}", highestFitness);
		std::string txt3 = "Average Fitness: " + std::format("{:.2f}", averageFitness);
		std::string txt4 = "Lowest Fitness: " + std::format("{:.2f}", lowestFitness);
		DrawText(txt2.c_str(), 25, 70, 45, WHITE);
		DrawText(txt3.c_str(), 25, 110, 45, WHITE);
		DrawText(txt4.c_str(), 25, 150, 45, WHITE);
		doGenerationButton.Update();
		doGenerationButton.Draw();
		doGenerationsButton.Update();
		doGenerationsButton.Draw();
		runBest.Update();
		runBest.Draw();
		runRand.Update();
		runRand.Draw();

		if (doGenerationsButton.Pressed())
		{
			lappingGenerations = true;
		}
		if (doGenerationButton.Pressed() || lappingGenerations)
		{
			testOrganism.running = false;
			DrawRectangle(0, 0, 1000, 600, { 0,0,0, 80 });
			DrawText("...", 25, 300, 50, WHITE);
			if (lappingGenerations) {
				if (IsKeyDown(KEY_X))
					lappingGenerations = false;
				DrawText("HOLD X TO STOP", 25, 350, 50, WHITE);
			}

			DrawSnakeGame(systems[0], Coord{ 580, 185 }, 400);
			EndDrawing();
			evolver.EvaluateGeneration();
			testOrganism.system = startSystem;

			
			
			organisms = evolver.GetOrganisms();
			highestFitness = -10000000000000000000.0f;
			averageFitness = 0;
			lowestFitness = 10000000000000000000.0f;
			for (int i = 0; i < evolver.GetPopulation(); i++)
			{
				averageFitness += organisms[i].fitness;
				lowestFitness = std::min(organisms[i].fitness, lowestFitness);
				highestFitness = std::max(organisms[i].fitness, highestFitness);
			}
			averageFitness /= evolver.GetPopulation();
			continue;
		}
		if (runBest.Pressed())
		{
			//clone network from best organism
			testOrganism.network = evolver.FindBestOrganism().GetNetwork();
			//set snake values to the same as the initial values for this generation
			testOrganism.system = startSystem;
			testOrganism.steps = 0;
			testOrganism.stepTimer = 0;
			testOrganism.fitness = 0;
			testOrganism.running = true;

			SetNetworkInputs(testOrganism.system, testOrganism.inputs);
		}
		if (runRand.Pressed())
		{
			//clone network from best organism
			int randomIndex = dist(random) * (POPULATION_SIZE / (GRID_SIZE - 1));
			testOrganism.network = evolver.GetOrganisms()[randomIndex].GetNetwork();
			//set snake values to the same as the initial values for this generation
			testOrganism.system = startSystem;
			testOrganism.steps = 0;
			testOrganism.stepTimer = 0;
			testOrganism.fitness = 0;
			testOrganism.running = true;

			SetNetworkInputs(testOrganism.system, testOrganism.inputs);
		}
		
		if (testOrganism.running)
		{
			testOrganism.stepTimer += deltaTime;
			if (testOrganism.stepTimer >= STEP_TIME || (IsKeyDown(KEY_SPACE) && testOrganism.stepTimer >= 0.01f) 
				|| (IsKeyDown(KEY_LEFT_CONTROL) && testOrganism.stepTimer >= 0.003f)
				|| (IsKeyDown(KEY_LEFT_ALT)))
			{
				if (testOrganism.steps > MAX_STEPS)
					testOrganism.running = false;

				testOrganism.stepTimer = 0;
				testOrganism.network.Evaluate(testOrganism.inputs, 4);
				StepOrganism(testOrganism.system, testOrganism.network.GetPreviousActivations(), testOrganism.fitness, testOrganism.running);
				SetNetworkInputs(testOrganism.system, testOrganism.inputs);
				testOrganism.steps++;
			}
		}

		txt2 = "Steps: " + std::to_string(testOrganism.steps);
		txt3 = "Fitness: " + std::format("{:.2f}", testOrganism.fitness);
		txt4 = "Steps left: " + std::to_string(testOrganism.system.stepsLeft);
		DrawText(txt2.c_str(), 400, 525, 20, WHITE);
		DrawText(txt3.c_str(), 400, 545, 20, WHITE);
		DrawText(txt4.c_str(), 400, 565, 20, WHITE);
		DrawSnakeGame(testOrganism.system, Coord{ 580, 185 }, 400);
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
	
	ptr->SetupStartSystem();
	SnakeSystem& startSystem = ptr->startSystem;
	

	for (size_t i = 0; i < POPULATION_SIZE; i++)
	{
		//setup initial values for every snake system (all the same)
		ptr->systems[i] = startSystem;
		//set network inputs
		SetNetworkInputs(ptr->systems[i], organisms[i].GetNetworkInputArray());
	}
}

void SnakeGame::StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex)
{
	auto* game = (SnakeGame*)evolver.GetUserPointer();
	auto& sys = game->systems[organismIndex];
	game->StepOrganism(sys, organism.GetNetworkOutputActivations(), organism.fitness, organism.continueStepping);
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
		isOnLeft |= left == bodyPart;
		isOnRight |= right == bodyPart;
		isInFront |= forward == bodyPart;
	}

	//if there is a bodypart or wall on left side of head
	inputs[0] = (float)isOnLeft;
	//if there is a bodypart or wall on right side of head
	inputs[1] = (float)isOnRight;
	//if there is a bodypart or wall in front of head
	inputs[2] = (float)isInFront;

	//angle to apple from current facing direction
	Coord direction = forward - head;
	Vector2 appleDir = { system.appleCoord.x - head.x, system.appleCoord.y - head.y };
	float invSq = 1.0f / sqrtf(appleDir.x * appleDir.x + appleDir.y * appleDir.y);
	appleDir.x *= invSq;
	appleDir.y *= invSq;
	//input 3 is the dot product of the direction to the apple and the perpendicular direction of the head
	inputs[3] = direction.y * appleDir.x - direction.x * appleDir.y;
}

void SnakeGame::StepOrganism(SnakeSystem& system, const float* networkOutputs, float& fitness, bool& continueStepping)
{
	//read output actions
	char maxIndex = 0;
	if (networkOutputs[1] > networkOutputs[0])
		maxIndex = 1;
	if (networkOutputs[2] > networkOutputs[maxIndex])
		maxIndex = 2;

	char moveDir = (char)system.movementDirection;
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
	//loop direction so it doesn't go out of bounds
	moveDir %= (char)Direction::COUNT;
	if (moveDir < 0)
		moveDir += (char)Direction::COUNT;
	system.movementDirection = (SnakeGame::Direction)moveDir;

	//now movement direction is set up correctly, step the game
	switch (StepSystem(system))
	{
	case SystemState::NOTHING:
		fitness += FITNESS_GAIN;
		break;
	case SystemState::GAME_OVER:
		continueStepping = false;
		return;
	case SystemState::APPLE_GET:
		system.stepsLeft += APPLE_GAIN;
		break;
	}

	system.stepsLeft--;
	if (system.stepsLeft <= 0)
	{
		continueStepping = false;
	}
}

SnakeGame::SystemState SnakeGame::StepSystem(SnakeSystem& system)
{
	//move snake in the movement direction
	Coord newCoord = AddDirectionToCoord(system.body[0], system.movementDirection);

	//if head intersects with body pos, game over
	for (Coord pos : system.body)
		if (pos == newCoord)
			return SystemState::GAME_OVER;

	//if head outside of bounds, game over
	if (newCoord.x >= GRID_SIZE || newCoord.x < 0 || newCoord.y >= GRID_SIZE || newCoord.y < 0)
		return SystemState::GAME_OVER;


	//if no die add head to body
	system.body.insert(system.body.begin(), newCoord);
	//if head intersects with apple place apple in new place
	if (newCoord.x == system.appleCoord.x && newCoord.y == system.appleCoord.y)
	{
		//place apple in new place
		bool appleIntersecting = false;
		//loop apple placement until it isn't on body
		do {
			appleIntersecting = false;
			system.appleCoord = Coord(dist(system.random), dist(system.random));
			for (Coord pos : system.body)
				appleIntersecting |= pos == system.appleCoord;
		} while (appleIntersecting);
		return SystemState::APPLE_GET;
	}
	else
		system.body.pop_back();

	return SystemState::NOTHING;
}

SnakeGame::Direction SnakeGame::GetLocalDirection(Direction direction, Direction relativeDirection)
{
	char dir = (char)direction;
	switch (relativeDirection)
	{
	case SnakeGame::Direction::LEFT:
		dir++;
		break;
	case SnakeGame::Direction::RIGHT:
		dir--;
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

Coord SnakeGame::AddDirectionToCoord(Coord coord, Direction direction)
{
	switch (direction)
	{
	case SnakeGame::Direction::LEFT:
		coord.x--;
		break;
	case SnakeGame::Direction::UP:
		coord.y++;
		break;
	case SnakeGame::Direction::RIGHT:
		coord.x++;
		break;
	case SnakeGame::Direction::DOWN:
		coord.y--;
		break;
	}
	return coord;
}

void SnakeGame::SetupStartSystem()
{
	//the same every time:
	static int seed = random();
	startSystem.random.seed(seed);
	//not the same every time
	//startSystem.random.seed(random());

	startSystem.body.clear();
	startSystem.movementDirection = Direction::DOWN;
	startSystem.body.emplace_back(GRID_SIZE / 2, GRID_SIZE / 2);
	startSystem.body.push_back(startSystem.body[0] + Coord(0, 1));
	startSystem.body.push_back(startSystem.body[0] + Coord(0, 2));
	startSystem.stepsLeft = STARTING_STEPS;

	bool appleIntersecting = false;
	//loop apple placement until it isn't on body
	do {
		appleIntersecting = false;
		//the same every time:
		startSystem.appleCoord = Coord{ 4, 4 };
		//not the same every time
		//startSystem.appleCoord = Coord{ dist(random), dist(random) };
		for (Coord pos : startSystem.body)
			appleIntersecting |= pos == startSystem.appleCoord;
	} while (appleIntersecting);
}

void SnakeGame::DrawSnakeGame(SnakeSystem& system, Coord pos, short size)
{
	float cellSize = (float)size / (GRID_SIZE + 2);
	Vector2 bottomLeft = { pos.x + cellSize, pos.y + cellSize };
	//draw border
	DrawRectangle(pos.x, pos.y, size, size, DARKGRAY);
	DrawRectangle(pos.x + cellSize, pos.y + cellSize, size - cellSize * 2, size - cellSize * 2, BLACK);

	//draw apple
	DrawRectangle(bottomLeft.x + cellSize * system.appleCoord.x, bottomLeft.y + cellSize * system.appleCoord.y,
		cellSize, cellSize, RED);

	DrawRectangle(bottomLeft.x + cellSize * 30, bottomLeft.y + cellSize * 15,
		cellSize, cellSize, GRAY);
	DrawRectangle(bottomLeft.x + cellSize * -1, bottomLeft.y + cellSize * 15,
		cellSize, cellSize, GRAY);
	DrawRectangle(bottomLeft.x + cellSize * 15, bottomLeft.y + cellSize * 30,
		cellSize, cellSize, GRAY);
	DrawRectangle(bottomLeft.x + cellSize * 15, bottomLeft.y + cellSize * -1,
		cellSize, cellSize, GRAY);

	for (auto bodyPart : system.body)
	{
		DrawRectangle(bottomLeft.x + cellSize * bodyPart.x, bottomLeft.y + cellSize * bodyPart.y,
			cellSize, cellSize, LIGHTGRAY);
	}
	if (system.body.size() > 0)
	{
		DrawRectangle(bottomLeft.x + cellSize * system.body[0].x, bottomLeft.y + cellSize * system.body[0].y,
			cellSize, cellSize, GRAY);

		Vector2 dir = {0,0};
		switch (system.movementDirection)
		{
		case Direction::UP:
			dir.y++;
			break;
		case Direction::DOWN:
			dir.y--;
			break;
		case Direction::LEFT:
			dir.x--;
			break;
		case Direction::RIGHT:
			dir.x++;
			break;
		}
		Vector2 midPoint = { bottomLeft.x + cellSize * system.body[0].x + cellSize/2.0f , bottomLeft.y + cellSize * system.body[0].y + cellSize / 2.0f };
		Vector2 addition = { dir.x * (cellSize / 2.0f), dir.y * (cellSize / 2.0f) };
		DrawLine(midPoint.x + addition.x, midPoint.y + addition.y, midPoint.x - addition.x, midPoint.y - addition.y, RED);
		DrawCircle(midPoint.x + addition.x, midPoint.y + addition.y, 3, RED);

	}
	

}
