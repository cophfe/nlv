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
			//setup deltatime
			std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
			deltaTime = (float)std::chrono::duration_cast<std::chrono::duration<double>>(start - lastTime).count();
			lastTime = start;

			DrawEvolverWindow();
			DrawDataWindow();
			DrawPlayWindow();
		}
		app.EndFrame();
	}
}

FlappyBird::FlappyBird()
{
	seed = time(0);
	
	random.seed(seed);

	app.Setup(1000, 700, "hello there");
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
	if (evolverIsSetup || evolverIsRunning)
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

	if (evolverIsSetup)
	{
		ImGui::EndDisabled();

		if (ImGui::Button("Restart and Reconfigure"))
		{
			ImGui::OpenPopup("Reconfigure Evolver?");
		}
		if (ImGui::BeginPopup("Reconfigure Evolver?"))
		{
			if (ImGui::Button("Confirm"))
			{
				//allow user to reconfigure evolver
				evolver = NetworkEvolver();
				SetupStartSystem();
				testOrganism.system = templateSystem;
				evolverIsSetup = false;
				ImGui::CloseCurrentPopup();
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

		if (evolverIsRunning)
			ImGui::EndDisabled();
	}

	//Episode
	ImGui::Separator();
	ImGui::Text("These values can be modified after construction");
	{
		ImGui::BeginDisabled(evolverIsRunning);

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

		ImGui::EndDisabled();
	}

	ImGui::End();
}


void FlappyBird::DrawDataWindow()
{
	ImGui::Begin("Data");

	ImGui::BeginDisabled(!evolverIsSetup);
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

	if (evolver.GetIsInitiated())
	{
		auto pop = evolver.GetPopulationArray();
		float perRow = glm::sqrt((float)evolver.GetPopulationSize());
		float width = ImGui::GetWindowWidth();
		
		for (int i = 0; i < evolver.GetPopulationSize(); i++)
		{
			for (int x = 0; x < perRow, i < evolver.GetPopulationSize(); x++, i++)
			{
				if (x > 0)
					ImGui::SameLine();
				ImGui::PushID(i);
				if (ImGui::Selectable(std::format("%.2f", pop[i].GetFitness()).c_str(), false, 0, ImVec2(width/ perRow, width / perRow)))
				{

				}
				ImGui::PopID();
			}
		}
	}

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
		//but this is just a progress bar so I'm not making sure its accurate at the cost of performance. it works surprisingly well given the fact it is terrible
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
	testOrganism.system = templateSystem;

	averages.clear();
	maximums.clear();
	minimums.clear();
	maxEver = 100;
	minEver = 0;
}
