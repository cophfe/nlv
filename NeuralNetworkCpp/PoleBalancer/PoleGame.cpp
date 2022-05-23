#include "PoleGame.h"

void PoleGame::Run()
{
	Network network(4, { 2 }, 1);
	NetworkEvolverDefinition def(network, POPULATION_SIZE, MAX_STEPS, 0.02f, 0.1f, StepFunction, EvolverMutationType::Add, EvolverCrossoverType::Point,
		EvolverSelectionType::Ranked, OnStartGeneration, nullptr, false, false, time(0), 30);
	NetworkEvolver evolver = NetworkEvolver(def);
	evolver.SetUserPointer(this);
	evolver.EvaluateGeneration();

	Button doGenerationButton(Coord(25, 200), Coord(250, 50), RED, MAROON, WHITE, 30, 10, "Do Generation", nullptr);
	Button doGenerationsButton(Coord(25, 260), Coord(250, 50), RED, MAROON, WHITE, 30, 10, "Do Generations", nullptr);
	Button runBest(Coord(730, 50), Coord(250, 50), RED, MAROON, WHITE, 30, 10, "Run Best", nullptr);
	Button runRand(Coord(730, 120), Coord(250, 50), RED, MAROON, WHITE, 30, 10, "Run Random", nullptr);

	SetupStartSystem();

	struct {
		Network network;
		PoleSystem system;
		float fitness = 0;
		float continueTimer = 0;
		unsigned int steps = 0;
		unsigned int orgIndex = 0;
		float inputs[4];
		bool running = false;
		
	} testOrganism;
	testOrganism.system = templateSystem;

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

			DrawGame(systems[0], 580, 185 , 400, 400, 50);
			EndDrawing();
			evolver.EvaluateGeneration();
			testOrganism.system = templateSystem;
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
			auto* organisms = evolver.GetOrganisms();
			float fitness = organisms[0].fitness;
			testOrganism.orgIndex = 0;
			for (size_t i = 1; i < evolver.GetPopulation(); i++)
			{
				if (organisms[i].fitness > fitness)
				{
					fitness = organisms[i].fitness;
					testOrganism.orgIndex = i;
				}
			}
			testOrganism.network = evolver.GetOrganisms()[testOrganism.orgIndex].GetNetwork();
			//set snake values to the same as the initial values for this generation
			testOrganism.system = templateSystem;
			testOrganism.fitness = 0;
			testOrganism.steps = 0;
			testOrganism.continueTimer = 0;
			testOrganism.running = true;
			SetNetworkInputs(testOrganism.system, testOrganism.inputs);
		}
		if (runRand.Pressed())
		{
			testOrganism.orgIndex = indexDist(random);
			testOrganism.network = evolver.GetOrganisms()[testOrganism.orgIndex].GetNetwork();
			//set snake values to the same as the initial values for this generation
			testOrganism.system = templateSystem;
			testOrganism.steps = 0;
			testOrganism.fitness = 0;
			testOrganism.running = true;
			SetNetworkInputs(testOrganism.system, testOrganism.inputs);
		}

		if (testOrganism.running)
		{
			testOrganism.continueTimer += deltaTime;
			if (testOrganism.continueTimer >= TIME_STEP || (IsKeyDown(KEY_SPACE) && testOrganism.continueTimer >= TIME_STEP / 2)
				|| (IsKeyDown(KEY_LEFT_CONTROL) && testOrganism.continueTimer >= TIME_STEP / 4)
				|| (IsKeyDown(KEY_LEFT_ALT)))
			{
				if (testOrganism.steps > MAX_STEPS)
					testOrganism.running = false;

				testOrganism.continueTimer = 0;
				testOrganism.network.Evaluate(testOrganism.inputs, 4);
				StepOrganism(testOrganism.system, *testOrganism.network.GetPreviousActivations(), testOrganism.fitness, testOrganism.running);
				SetNetworkInputs(testOrganism.system, testOrganism.inputs);
				testOrganism.steps++;
			}
		}

		txt2 = "Steps: " + std::to_string(testOrganism.steps);
		txt3 = "Fitness: " + std::format("{:.2f}", testOrganism.fitness);
		txt4 = "Org #: " + std::to_string(testOrganism.orgIndex);
		DrawText(txt2.c_str(), 400, 525, 20, WHITE);
		DrawText(txt3.c_str(), 400, 545, 20, WHITE);
		DrawText(txt4.c_str(), 400, 565, 20, WHITE);
		DrawGame(testOrganism.system, 580, 185, 400, 400, 50);
		EndDrawing();
	}
}

PoleGame::PoleGame()
{
	SetConfigFlags(FLAG_MSAA_4X_HINT);// | FLAG_VSYNC_HINT);
	InitWindow(1000, 600, "Snake Test");
	random.seed(time(0));
}

PoleGame::~PoleGame()
{
	CloseWindow();
}

void PoleGame::OnStartGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms)
{
	PoleGame* ptr = (PoleGame*)evolver.GetUserPointer();
	ptr->SetupStartSystem();
	auto& ts = ptr->templateSystem;
	
	for (size_t i = 0; i < POPULATION_SIZE; i++)
	{
		ptr->systems[i] = ts;
		SetNetworkInputs(ptr->systems[i], organisms[i].GetNetworkInputArray());
	}
}

void PoleGame::StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex)
{
	PoleGame* ptr = (PoleGame*)evolver.GetUserPointer();
	ptr->StepOrganism(ptr->systems[organismIndex], *organism.GetNetworkOutputActivations(), organism.fitness, organism.continueStepping);
}

void PoleGame::SetNetworkInputs(PoleSystem& system, float* inputs)
{
	inputs[0] = system.cartPosition;
	inputs[1] = system.cartVelocity;
	inputs[2] = system.poleAngle;
	inputs[3] = system.poleVelocity;
}

void PoleGame::StepOrganism(PoleSystem& system, float networkOutput, float& fitness, bool& continueStepping)
{
	//fitness is time
	fitness += TIME_STEP;
	
	float force = glm::sign(networkOutput - 0.5f) * FORCE;

	system.cartPosition += TIME_STEP * system.cartVelocity;
	system.cartVelocity += TIME_STEP * system.cartAcceleration;
	system.poleAngle += TIME_STEP * system.poleVelocity;
	system.poleVelocity += TIME_STEP * system.poleAcceleration;
	
	float iMass = 1.0f / (CART_MASS + POLE_MASS);
	float cosAngle = glm::cos(system.poleAngle);
	float sinAngle = glm::sin(system.poleAngle);

	system.cartAcceleration = (force + POLE_MASS * POLE_LENGTH * (system.poleVelocity * system.poleVelocity * sinAngle - system.poleAcceleration * cosAngle)) * iMass;
	
	system.poleAcceleration = GRAVITY * sinAngle + cosAngle *
		(-force - POLE_MASS * POLE_LENGTH * system.poleVelocity * system.poleVelocity * sinAngle * iMass);
	system.poleAcceleration /= POLE_LENGTH * (4.0f / 3.0f - POLE_MASS * cosAngle * cosAngle * iMass);

	if (glm::abs(system.poleAngle) >= POLE_FAILURE_ANGLE
		|| glm::abs(system.cartPosition) >= TRACK_LIMIT)
	{
		continueStepping = false;
	}
}

void PoleGame::SetupStartSystem()
{
	templateSystem.cartVelocity = 0;
	templateSystem.cartAcceleration = 0;
	templateSystem.poleVelocity = 0;
	templateSystem.poleAcceleration = 0;
	templateSystem.poleAngle = 0;// dist(random);
	templateSystem.cartPosition = 0;// dist(random);
}

void PoleGame::DrawGame(PoleSystem& system, short x, short y, short sizeX, short sizeY, short floorSize)
{
	DrawRectangle(x, y, sizeX, sizeY, DARKGRAY);
	unsigned short borderSize = std::min(sizeX / 100, 1);
	sizeX = sizeX - borderSize * 2;
	sizeY = sizeY - borderSize * 2;
	x += borderSize;
	y += borderSize;
	DrawRectangle(x, y, sizeX, sizeY, BLACK);

	float unit = (float)sizeX / (TRACK_LIMIT * 2.0f + 1);
	short cartSizeY = unit * 0.5f;
	//floor
	DrawRectangle(x, y + sizeY - floorSize, sizeX, floorSize, GRAY);
	//cart
	Vec2 cartStart = Vec2(x + sizeX / 2 - cartSizeY + unit * system.cartPosition, y + sizeY - floorSize - cartSizeY);
	DrawRectangle(cartStart.x, cartStart.y, cartSizeY * 2, cartSizeY, RED);
	Vec2 start = Vec2(cartStart.x + cartSizeY, cartStart.y);
	Vec2 offset =  (POLE_LENGTH * 2.0f * unit) * Vec2(glm::sin(system.poleAngle), glm::cos(system.poleAngle));
	DrawLine(start.x, start.y, start.x + offset.x, start.y - offset.y, WHITE);
}
