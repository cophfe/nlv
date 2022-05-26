#include "FlappyBird.h"

void FlappyBird::Run()
{
	lastTime = std::chrono::high_resolution_clock::now();

	while (!glfwWindowShouldClose(app.GetWindow()))
	{
		app.StartFrame();
		{
			//poll
			glfwPollEvents();
			//set deltatime
			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			deltaTime = (float)std::chrono::duration_cast<std::chrono::duration<double>>(start - lastTime).count();
			lastTime = start;

			//Draw UI
			DrawEvolverWindow();
			DrawDataWindow();
			DrawPlayWindow();

			//Run solution
			RunCurrentSolution();
		}
		app.EndFrame();
	}
}

FlappyBird::FlappyBird()
{
	seed = time(0);
	
	random.seed(seed);

	//setup rendering and callbacks
	app.SetupWindow(1000, 700, "hello there");
	glfwSetWindowUserPointer(app.GetWindow(), this);
	glfwSetKeyCallback(app.GetWindow(), OnKeyPressed);
	app.SetupImgui();

	//set evolver to default
	evolverIsSetup = true;
	ConfigureEvolver();
}

FlappyBird::~FlappyBird()
{
	app.UnSetup();
}

void FlappyBird::RunGeneration()
{
	if (evolver.GetIsInitiated() && !evolverIsRunning)
	{
		std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

		evolverIsRunning = true;
		evolver.EvaluateGeneration();
		evolverIsRunning = false;

		std::chrono::high_resolution_clock::time_point e = std::chrono::high_resolution_clock::now();
		timeToComplete = (float)std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start).count();

		GetEvolverValues();
	}
}

void FlappyBird::DrawEvolverWindow()
{
	//if  the evolver isn't setup, show setup UI
	ImGui::Begin("Configure");
	if (evolverIsSetup)
		ImGui::BeginDisabled();

	//show neural network information
	ImGui::Text("Input nodes: %i", INPUT_COUNT);
	if (ImGui::InputInt("Hidden layers", &hiddenLayers, 1, 1, ImGuiInputTextFlags_AlwaysOverwrite))
	{
		hiddenLayers = glm::clamp(hiddenLayers, 1, 5);
		nodesPerLayer.resize(hiddenLayers);
		for (size_t i = 0; i < hiddenLayers; i++)
		{
			nodesPerLayer[i] = glm::clamp(nodesPerLayer[i], 1, 100);
			
		}
	}
	ImGui::InputScalarN("Nodes per layer", ImGuiDataType_S32, nodesPerLayer.data(), hiddenLayers);

	ImGui::Text("Output nodes: 1");
	ImGui::SliderInt("Population", &populationSize, 100, 5000, "%d", ImGuiSliderFlags_Logarithmic);
	ImGui::Spacing();

	if (ImGui::Button("Reset to default"))
	{
		hiddenLayers = 1;
		nodesPerLayer = { DEFAULT_HIDDEN_NODES };
		populationSize = DEFAULT_POPULATION;
	}
	ImGui::Spacing();

	bool disabledWhileRunning = false;
	if (evolverIsSetup)
	{
		ImGui::EndDisabled();

		if (evolverIsRunning)
		{
			disabledWhileRunning = true;
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Restart and Reconfigure"))
		{
			if (evolver.GetGeneration() == 0 || !evolver.GetIsInitiated())
			{
				//allow user to reconfigure evolver
				evolver = NetworkEvolver();
				SetupStartSystem();
				currentSolution.system = templateSystem;
				evolverIsSetup = false;
				averages.clear();
				maximums.clear();
				minimums.clear();
				maxEver = 100;
				minEver = 0;
			}
			else
			{
				ImGui::OpenPopup("Reconfigure Evolver?");
			}
		}
		if (ImGui::BeginPopup("Reconfigure Evolver?"))
		{
			
			if (ImGui::Button("Confirm"))
			{
				//allow user to reconfigure evolver
				evolver = NetworkEvolver();
				SetupStartSystem();
				currentSolution.system = templateSystem;
				evolverIsSetup = false;
				ImGui::CloseCurrentPopup();
				averages.clear();
				maximums.clear();
				minimums.clear();
				maxEver = 100;
				minEver = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
	else
	{
		if (ImGui::Button("Construct"))
		{
			evolverIsSetup = true;
			ConfigureEvolver();
		}
	}

	
	//Episode
	ImGui::Separator();
	ImGui::Text("These values can be modified after construction");
	{

		if (ImGui::Checkbox("Multithread episodes", &multithread))
			evolver.SetIsThreadedEpisodes(multithread);
		if (ImGui::Checkbox("Constant seed", &staticEpisodes))
			evolver.SetStaticEpisodes(staticEpisodes);

		if (ImGui::SliderFloat("Max time", &maxTime, 10, 180, "%0.2f"))
		{
			maxSteps = maxTime / TIME_STEP;
			evolver.SetMaxSteps(maxSteps);
		}
		if (ImGui::SliderFloat("Elite percent", &elitePercent, 0, 1, "%0.2f"))
		{
			evolver.SetElitePercent(elitePercent);
		}
		//Mutation
		//Crossover
		//Selection

		if (ImGui::Button("Reset to default"))
		{
			maxTime = DEFAULT_MAX_TIME;
			maxSteps = DEFAULT_MAX_TIME / TIME_STEP;
			elitePercent = DEFAULT_ELITE;
			multithread = DEFAULT_THREADED;
			staticEpisodes = DEFAULT_STATIC;
		}

	}
	if (disabledWhileRunning)
		ImGui::EndDisabled();
	ImGui::End();
}


void FlappyBird::DrawDataWindow()
{
	ImGui::Begin("Data");

	ImGui::BeginDisabled(!evolverIsSetup || evolverIsRunning);
	if (ImGui::Button("Run Generation"))
	{
		std::thread thread(&FlappyBird::RunGeneration, this);
		thread.detach();
	}
	if (!evolverIsSetup)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("Create Evolver First");
	}
	else {
		ImGui::SameLine();
		ImGui::Text("Generation #%i", evolver.GetGeneration());
	}
	ImGui::EndDisabled();

	ImGui::ProgressBar(progress);
	ImGui::Text("Completion Time: %0.2f", timeToComplete);

	ImGui::Separator();
	
	if (ImPlot::BeginPlot("Generation Fitness"))
	{
		ImPlot::SetupAxes("Generation", "Fitness");

		if (evolver.GetGeneration() != 0)
		{
			ImPlot::SetupAxisLimits(ImAxis_X1, 1, evolver.GetGeneration() + 1, ImPlotCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, minEver - 1, maxEver + 1, ImPlotCond_Always);
		}
		ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
		ImPlot::PlotLine("Average", averages.data(), averages.size(), 1, 1, 1);
		ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
		ImPlot::PlotLine("Maximum", maximums.data(), maximums.size(), 1, 1, 1);
		ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
		ImPlot::PlotLine("Minimum", minimums.data(), minimums.size(), 1, 1, 1);
		ImPlot::EndPlot();
	}
	
	ImGui::End();
}

void FlappyBird::DrawPlayWindow()
{
	ImGui::Begin("Test");

	bool beginDisabled;
	bool beginSolutionDisabled = false;
	if (beginDisabled = evolverIsRunning || !evolverIsSetup)
		ImGui::BeginDisabled();

	ImGui::Text("Preview AI");
	if (ImGui::Button("Random"))
	{
		std::uniform_int_distribution<int> dist(0, evolver.GetPopulationSize() - 1);
		SetCurrentSolution(dist(random));
	}
	ImGui::SameLine();
	if (ImGui::Button("Best"))
	{
		const NetworkOrganism* organisms = evolver.GetPopulationArray();
		float fitness = organisms[0].fitness;
		int orgIndex = 0;
		for (size_t i = 1; i < evolver.GetPopulationSize(); i++)
		{
			if (organisms[i].fitness > fitness)
			{
				fitness = organisms[i].fitness;
				orgIndex = i;
			}
		}
		SetCurrentSolution(orgIndex);
	}
	ImGui::SameLine();
	if (ImGui::Button("Choose"))
	{
		ImGui::OpenPopup("Choose Index");
	}
	if (ImGui::BeginPopup("Choose Index"))
	{
		auto pop = evolver.GetPopulationArray();
		float width = 500;
		ImVec2 size = ImVec2(50, 25);
		int xSize = width / size.x;
		
		for (int i = 0; i < evolver.GetPopulationSize(); i++)
		{
			for (int x = 0; x < xSize && i < evolver.GetPopulationSize(); x++, i++)
			{
				if (x > 0)
					ImGui::SameLine();
				ImGui::PushID(i);

				auto formatted = std::format("{:.1f}\n", pop[i].GetFitness());

				float success = (pop[i].GetFitness() - minEver)/ (maxEver - minEver);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1 - success, success, 0, 1.0f));
				if (ImGui::Selectable(formatted.c_str(), false, 0, size))
				{
					SetCurrentSolution(i);
					ImGui::CloseCurrentPopup();
				}
				ImGui::PopStyleColor();
				ImGui::PopID();
			}
		}
		ImGui::EndPopup();
	}

	ImGui::Separator();
	ImGui::Text("Play Game");
	if (ImGui::Button("Start"))
	{
		SetCurrentSolutionToPlayMode();
	}
	ImGui::Separator();
	
	if (currentSolution.running)
		ImGui::Text("Running!");
	else
	{
		beginSolutionDisabled = true;
		ImGui::BeginDisabled();
		ImGui::Text("Not running");
	}
	
	if (ImGui::BeginTable("speed", 4, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
	{
		ImGui::TableNextColumn();
		if (ImGui::Selectable("Pause", currentSolution.playSpeed == 0))
		{
			currentSolution.playSpeed = 0;
		}
		ImGui::TableNextColumn();
		if (ImGui::Selectable("Play", currentSolution.playSpeed == 1))
		{
			currentSolution.playSpeed = 1;
		}
		ImGui::TableNextColumn();
		if (ImGui::Selectable(">", currentSolution.playSpeed == 2))
		{
			currentSolution.playSpeed = 2;
		}
		ImGui::TableNextColumn();
		if (ImGui::Selectable(">>", currentSolution.playSpeed == 3))
		{
			currentSolution.playSpeed = 3;
		}
		ImGui::EndTable();
	}

	if (currentSolution.isAI)
	{
		ImGui::Text("Current AI selected: #%i", currentSolution.orgIndex);
		ImGui::Text("Fitness: %f", currentSolution.fitness);
		ImGui::Text("Step: %i / %i", currentSolution.steps, maxSteps);
	}
	else
	{
		ImGui::Text("Manual control selected");
		ImGui::Text("Fitness: %f", currentSolution.fitness);
		ImGui::Text("Step: %i", currentSolution.steps, maxSteps);
	}
	
	if (beginDisabled)
		ImGui::EndDisabled();
	if (beginSolutionDisabled)
		ImGui::EndDisabled();

	ImGui::End();
}

void FlappyBird::GetEvolverValues()
{
	float average = 0;
	float min = 10000000000.0f;
	float max = -10000000000.0f;
	const NetworkOrganism* organisms = evolver.GetPopulationArray();
	for (int i = 0; i < evolver.GetPopulationSize(); i++)
	{
		average += organisms[i].fitness;
		min = std::min(organisms[i].fitness, min);
		max = std::max(organisms[i].fitness, max);
	}
	average /= evolver.GetPopulationSize();

	averages.push_back(average);
	maximums.push_back(max);
	minimums.push_back(min);
	maxEver = std::max(max, maxEver);
	minEver = std::min(min, minEver);
}

void FlappyBird::OnStartGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms)
{
	FlappyBird* ptr = (FlappyBird*)evolver.GetUserPointer();
	ptr->SetupStartSystem();
	auto& ts = ptr->templateSystem;
	
	for (size_t i = 0; i < evolver.GetPopulationSize(); i++)
	{
		ptr->systems[i] = ts;
		SetNetworkInputs(ptr->systems[i], organisms[i].GetNetworkInputArray());
	}

	ptr->progress = 0;
}

void FlappyBird::OnEndGeneration(const NetworkEvolver& evolver, NetworkOrganism* organisms)
{
	FlappyBird* ptr = (FlappyBird*)evolver.GetUserPointer();
	ptr->progress = 1;
}

void FlappyBird::StepFunction(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex)
{
	FlappyBird* ptr = (FlappyBird*)evolver.GetUserPointer();
	ptr->StepOrganism(ptr->systems[organismIndex], *organism.GetNetworkOutputActivations(), organism.fitness, organism.continueStepping);
	SetNetworkInputs(ptr->systems[organismIndex], organism.GetNetworkInputArray());

	if (organism.GetStepsTaken() >= evolver.GetMaxSteps() - 1 || !organism.continueStepping)
	{
		//r-r-r-race condition!
		//but this is just a progress bar sooo whatever it dont matter
		ptr->progress += 1.0f / ptr->populationSize;
	}

}

void FlappyBird::SetNetworkInputs(BirdSystem& system, float* inputs)
{
	
}

void FlappyBird::StepOrganism(BirdSystem& system, float networkOutput, float& fitness, bool& continueStepping)
{
	fitness += 0.02f;
}

void FlappyBird::SetupStartSystem()
{
	if (staticEpisodes)
	{

	}
	else
	{

	}
}

void FlappyBird::SetCurrentSolution(int organismIndex)
{
	const NetworkOrganism& org = evolver.GetPopulationArray()[organismIndex];
	currentSolution.network = org.GetNetwork();
	currentSolution.system = templateSystem;
	currentSolution.fitness = 0;
	currentSolution.steps = 0;
	currentSolution.continueTimer = 0;
	currentSolution.manualJump = false;
	currentSolution.playSpeed = 0;
	currentSolution.isAI = true;
	SetNetworkInputs(currentSolution.system, currentSolution.inputs);
	currentSolution.running = true;
}

void FlappyBird::SetCurrentSolutionToPlayMode()
{
	currentSolution.system = templateSystem;
	currentSolution.fitness = 0;
	currentSolution.steps = 0;
	currentSolution.continueTimer = 0;
	currentSolution.manualJump = false;
	currentSolution.playSpeed = 0;
	currentSolution.isAI = false;
	currentSolution.running = true;
}

void FlappyBird::RunCurrentSolution()
{
	if (currentSolution.running)
	{
		switch (currentSolution.playSpeed)
		{
			//case 0 is paused

		case 1: //Regular
			currentSolution.continueTimer += deltaTime;
			break;
		case 2: //Faster
			currentSolution.continueTimer += 5 * deltaTime;
			break;
		case 3: //Supa Fast
			currentSolution.continueTimer += 15 * deltaTime;
			break;
		}

		if (currentSolution.continueTimer >= TIME_STEP)
		{
			currentSolution.continueTimer = 0;

			if (currentSolution.isAI)
			{
				currentSolution.network.Evaluate(currentSolution.inputs, INPUT_COUNT);
				StepOrganism(currentSolution.system, *currentSolution.network.GetPreviousActivations(), currentSolution.fitness, currentSolution.running);
				SetNetworkInputs(currentSolution.system, currentSolution.inputs);

				currentSolution.steps++;
				if (currentSolution.steps >= maxSteps - 1)
				{
					currentSolution.running = false;
					return;
				}
			}
			else
			{
				StepOrganism(currentSolution.system, (float)currentSolution.manualJump, currentSolution.fitness, currentSolution.running);
				currentSolution.manualJump = false;
				currentSolution.steps++;
			}


		}
	}
}

void FlappyBird::ConfigureEvolver()
{
	Network network(INPUT_COUNT, nodesPerLayer, 1);
	EvolverBuilder def = EvolverBuilder(network, StepFunction, populationSize, maxSteps, seed)
		.SetMutation(EvolverMutationType::Add, 0.2f, 1.0f)
		.SetCrossover(EvolverCrossoverType::Uniform)
		.SetSelection(EvolverSelectionType::FitnessProportional)
		.SetCallbacks(OnStartGeneration, OnEndGeneration)
		.SetElitePercent(elitePercent)
		.SetEpisodeParameters(staticEpisodes, multithread, THREAD_COUNT);
	evolver = def.Build();
	systems = std::vector<BirdSystem>(populationSize);

	SetupStartSystem();
	evolver.SetUserPointer(this);
	currentSolution.system = templateSystem;

	averages.clear();
	maximums.clear();
	minimums.clear();
	maxEver = 100;
	minEver = 0;
}

void FlappyBird::OnKeyPressed(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	FlappyBird* ptr = (FlappyBird*)glfwGetWindowUserPointer(window);

	if (ptr->currentSolution.running && !ptr->currentSolution.isAI && action == GLFW_PRESS && keycode == GLFW_KEY_SPACE)
	{
		ptr->currentSolution.manualJump = true;
	}
}
