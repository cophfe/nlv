#include "NetworkEvolver.h"
#include <thread>
#include <cmath>
#include <fstream>

namespace nlv 
{
	NetworkEvolver::NetworkEvolver(const NetworkEvolverBuilder& def)
		: populationSize(def.populationSize), maxSteps(def.maxSteps), elitePercent(def.elitePercent),
		mutationRate(def.mutationRate), stepCallback(def.stepFunction), startCallback(def.startFunction), endCallback(def.endFunction),
		mutationType(def.mutationType), selectionType(def.selectionType), crossoverType(def.crossoverType), currentGeneration(0),
		threadedStepping(def.threadedEpisodes), episodeThreadCount(def.episodeThreadCount), staticEpisodes(def.staticEpisodes),
		mutationScale(def.mutationScale), userPointer(def.userPtr)
	{
		if (populationSize == 0)
			throw std::runtime_error("Generation size cannot be 0");
		if (maxSteps == 0)
			throw std::runtime_error("Max steps cannot be 0");
		if (stepCallback == nullptr)
			throw std::runtime_error("Step callback cannot be nullptr");

		neuralInputSize = def.networkTemplate.GetInputCount();
		neuralOutputSize = def.networkTemplate.GetOutputCount();

		//get an uninitialized array of organisms
		organisms = (NetworkOrganism*)(malloc(sizeof(NetworkOrganism) * populationSize));

		if (def.tournamentSize == 0)
			tournamentSize = std::clamp(populationSize / 50, 3U, 20U);
		else //tournamentSize cannot be less than three
			tournamentSize = std::min(def.tournamentSize, 3U);

		//setup index array used for creating next generations
		fitnessOrderedIndexes = new uint32_t[populationSize];
		for (size_t i = 0; i < populationSize; i++)
			fitnessOrderedIndexes[i] = i;

		if (def.seed == 0)
			random.engine.seed(std::random_device()());
		else
			random.engine.seed(def.seed);

		//initiate said array of organisms
		for (size_t i = 0; i < populationSize; i++)
		{
			new (organisms + i) NetworkOrganism(def.networkTemplate);
			organisms[i].network.RandomizeValues(random.engine);
		}

		initialized = true;
	}

	NetworkEvolver::~NetworkEvolver()
	{
		Uninitialize();
	}

	NetworkEvolver::NetworkEvolver(NetworkEvolver&& other)
		: populationSize(other.populationSize), maxSteps(other.maxSteps), elitePercent(other.elitePercent),
		mutationRate(other.mutationRate), stepCallback(other.stepCallback), startCallback(other.startCallback), endCallback(other.endCallback),
		mutationType(other.mutationType), selectionType(other.selectionType), crossoverType(other.crossoverType), currentGeneration(0),
		threadedStepping(other.threadedStepping), episodeThreadCount(other.episodeThreadCount), staticEpisodes(other.staticEpisodes),
		mutationScale(other.mutationScale), userPointer(other.userPointer)
	{
		neuralInputSize = other.neuralInputSize;
		neuralOutputSize = other.neuralOutputSize;
		organisms = other.organisms;
		tournamentSize = other.tournamentSize;
		fitnessOrderedIndexes = other.fitnessOrderedIndexes;
		random = other.random;
		initialized = other.initialized;
		activateStaticEpisodes = other.activateStaticEpisodes;

		other.populationSize = 0;
		other.organisms = nullptr;
		other.fitnessOrderedIndexes = nullptr;
		other.initialized = false;
	}

	NetworkEvolver& NetworkEvolver::operator=(NetworkEvolver&& other)
	{
		Uninitialize();

		populationSize = other.populationSize;
		maxSteps = other.maxSteps;
		elitePercent = other.elitePercent;
		mutationRate = other.mutationRate;
		stepCallback = other.stepCallback;
		startCallback = other.startCallback;
		endCallback = other.endCallback;
		mutationType = other.mutationType;
		selectionType = other.selectionType;
		crossoverType = other.crossoverType;
		currentGeneration = 0;
		threadedStepping = other.threadedStepping;
		episodeThreadCount = other.episodeThreadCount;
		staticEpisodes = other.staticEpisodes;
		mutationScale = other.mutationScale;
		userPointer = other.userPointer;
		neuralInputSize = other.neuralInputSize;
		neuralOutputSize = other.neuralOutputSize;
		organisms = other.organisms;
		tournamentSize = other.tournamentSize;
		fitnessOrderedIndexes = other.fitnessOrderedIndexes;
		random = other.random;
		initialized = other.initialized;
		activateStaticEpisodes = other.activateStaticEpisodes;

		other.populationSize = 0;
		other.organisms = nullptr;
		other.fitnessOrderedIndexes = nullptr;
		other.initialized = false;
		return *this;
	}

	void NetworkEvolver::CreateNewGen()
	{
		//the first generation hasn't been stepped yet, and therefore cannot be used for creating a new generation
		//the reason it is ordered CreateGen(), StepGen() is so the fitness data from the generation step is saved for the user to access without any copying
		//stepgen isn't called inside of the start function because that might confuse things for the user when initiating
		if (currentGeneration == 0)
			return;

		//Order:
		//Retain elite: elite get copied into next generation
		//Selection : select a number of parents
		//Crossover : crossover said parents to create a child
		//Loop: do selection and crossover until you have enough in the generation
		//Mutation : go over all new children and modify some genes

		//Make an index array ordered by the fitnesses of organisms
		std::sort(fitnessOrderedIndexes, fitnessOrderedIndexes + populationSize, [this](int a, int b) { return organisms[a].fitness > organisms[b].fitness; });

		//since I want to be able to save the network evolver and normal_distribution is stateful it needs to be reset before every generation to keep everything deterministic after save() and load()
		random.guassan.reset();
		//uniform_real_distribution probably doesn't use it's internal state but i'll call reset() on it just in case
		random.dist.reset();

		//create new organism array
		NetworkOrganism* newOrganisms = (NetworkOrganism*)(malloc(sizeof(NetworkOrganism) * populationSize));
		if (newOrganisms == nullptr)
			throw std::runtime_error("Cannot allocate new organism array.");
		memset(newOrganisms, 0, sizeof(NetworkOrganism) * populationSize);

		//Retain elite in next generation
		uint32_t eliteCount = std::min((uint32_t)(elitePercent * populationSize), populationSize);
		for (size_t i = 0; i < eliteCount; i++)
		{
			new (newOrganisms + i) NetworkOrganism(organisms[fitnessOrderedIndexes[i]]);
			if (!staticEpisodes)
				newOrganisms[i].Reset();
		}

		//the rest of the newGeneration will be populated with children of the previous generation
		int childIndex = eliteCount;

		//Select parents and crossover to create children
		//note: more than two parents can generate better genomes
		switch (selectionType)
		{
		case EvolverSelectionType::FitnessProportional:
		{
			//if there are negative fitnesses, add an addition to fitness values to make them all more than 0
			float fitnessAddition = -std::min(organisms[fitnessOrderedIndexes[populationSize - 1]].fitness, 0.0f);

			//float inverseTotalFitness = 0;
			//for (size_t i = 0; i < population; i++)
			//	inverseTotalFitness += organisms[i].fitness;
			//inverseTotalFitness += fitnessAddition * population;
			//inverseTotalFitness = 1.0f / (inverseTotalFitness);

			while (childIndex < populationSize)
			{
				NetworkOrganism& p1 = SelectionFitnessProportional(fitnessAddition);
				NetworkOrganism& p2 = SelectionFitnessProportional(fitnessAddition);

				new (newOrganisms + childIndex) NetworkOrganism(p1.network);
				Crossover(newOrganisms[childIndex], p1, p2);
				childIndex++;
			}
		}
		break;
		case EvolverSelectionType::Ranked:
		{
			//1 / guass formula
			float inverseSumOfAllRanks = 2.0f / (populationSize * (populationSize + 1));

			while (childIndex < populationSize)
			{
				NetworkOrganism& p1 = SelectionRanked(inverseSumOfAllRanks);
				NetworkOrganism& p2 = SelectionRanked(inverseSumOfAllRanks);

				new (newOrganisms + childIndex) NetworkOrganism(p1.network);
				Crossover(newOrganisms[childIndex], p1, p2);
				childIndex++;
			}
		}
		break;
		case EvolverSelectionType::Tournament:
		{
			std::vector<int> tournament(tournamentSize);
			while (childIndex < populationSize)
			{
				//get tournamentSize random indexes
				for (size_t i = 0; i < tournamentSize; i++)
					tournament[i] = random.ChanceIndex(populationSize);

				//get the two best from the tournament and use them as parents
				uint32_t best = 0;
				uint32_t secondBest = 0;
				float bestFitness = 0;
				float secondBestFitness = 0;

				for (size_t i = 0; i < tournamentSize; i++)
				{
					if (organisms[i].fitness >= bestFitness)
					{
						secondBestFitness = bestFitness;
						secondBest = best;
						best = i;
						bestFitness = organisms[i].fitness;
					}
					else if (organisms[i].fitness > secondBestFitness)
					{
						secondBestFitness = organisms[i].fitness;
						secondBest = i;
					}
				}

				NetworkOrganism* p1 = organisms + best;
				NetworkOrganism* p2 = organisms + secondBest;
				new (newOrganisms + childIndex) NetworkOrganism(p1->network);
				Crossover(newOrganisms[childIndex], *p1, *p2);
				childIndex++;
			}
		}
		break;
		case EvolverSelectionType::Custom:
		{
			if (!selectionCallback)
				throw std::runtime_error("Selection callback cannot be nullptr when selection type is custom");

			NetworkOrganism* p1;
			NetworkOrganism* p2;

			while (childIndex < populationSize)
			{
				selectionCallback(organisms, p1);
				selectionCallback(organisms, p2);

				new (newOrganisms + childIndex) NetworkOrganism(p1->network);
				Crossover(newOrganisms[childIndex], *p1, *p2);
				childIndex++;
			}
		}
		break;
		default:
			throw std::runtime_error("Selection type is incorrectly defined");
			break;
		}

		//Mutate new children
		switch (mutationType)
		{
		case EvolverMutationType::Set:
			for (size_t i = eliteCount; i < populationSize; i++)
			{
				while (random.Chance() < mutationRate)
					MutateSet(newOrganisms[i]);
			}
			break;
		case EvolverMutationType::Add:
			for (size_t i = eliteCount; i < populationSize; i++)
			{
				while (random.Chance() < mutationRate)
					MutateAdd(newOrganisms[i]);
			}
			break;
		case EvolverMutationType::Custom:
			for (size_t i = eliteCount; i < populationSize; i++)
			{
				if (!mutationCallback)
					throw std::runtime_error("Mutation callback cannot be nullptr when mutation type is custom");
				while (random.Chance() < mutationRate)
				{
					mutationCallback(organisms[i].network.genes, organisms[i]);
				}
			}
			break;
		default:
			throw std::runtime_error("Mutation type is incorrectly defined");
			break;
		}

		//destroy previous generation
		for (size_t i = 0; i < populationSize; i++)
			(*(organisms + i)).~NetworkOrganism();
		free(organisms);

		//set organisms to the new generation
		organisms = newOrganisms;
	}

	NetworkOrganism& NetworkEvolver::SelectionFitnessProportional(float fitnessAddition)
	{
		//stochastic acceptance based (faster generally)
		uint32_t parentIndex;
		float inverseLargestFitness = 1.0f / (organisms[fitnessOrderedIndexes[0]].fitness + fitnessAddition);
		do {
			parentIndex = random.ChanceIndex(populationSize);
		} while (random.Chance() > (organisms[parentIndex].fitness + fitnessAddition) * inverseLargestFitness);
		return organisms[parentIndex];

		//non stochastic acceptance based
		//float probability = 0;
		//float chance = random.Chance();
		//for (size_t i = 0; i < population; i++)
		//{
		//	probability += ((organisms[fitnessOrderedIndexes[i]].fitness + fitnessAddition) * inverseTotalFitness);
		//	if (chance <= probability)
		//		return organisms[i];
		//}
		////otherwise something went wrong, but this will surely never happen right?
		//return organisms[fitnessOrderedIndexes[population - 1]];
	}

	NetworkOrganism& NetworkEvolver::SelectionRanked(float inverseSumOfAllRanks)
	{
		float probability = 0;
		float chance = random.Chance();
		for (size_t i = 0; i < populationSize; i++)
		{
			probability += (populationSize - (i + 1)) * inverseSumOfAllRanks;
			if (chance <= probability)
				return organisms[fitnessOrderedIndexes[i]];
		}
		//otherwise something went wrong, but this will surely never happen right?
		return organisms[fitnessOrderedIndexes[populationSize - 1]];

	}

	void NetworkEvolver::Crossover(NetworkOrganism& child, NetworkOrganism& p1, NetworkOrganism& p2)
	{
		//note: assumes child's network is a clone of p1 already
		switch (crossoverType)
		{
		case EvolverCrossoverType::Uniform:
		{
			for (size_t i = 0; i < child.network.geneCount; i++)
			{
				if (random.Chance() > 0.5f)
					child.network.genes[i] = p2.network.genes[i];
			}
		}
		break;
		case EvolverCrossoverType::Point:
		{
			uint32_t point = random.ChanceIndex(child.network.geneCount);
			memcpy(child.network.genes + point, p2.network.genes + point, (child.network.geneCount - point) * sizeof(float));
		}
		break;
		case EvolverCrossoverType::TwoPoint:
		{
			uint32_t point1 = random.ChanceIndex(child.network.geneCount);
			uint32_t point2 = random.ChanceIndex(child.network.geneCount);
			uint32_t size;
			if (point1 > point2)
			{
				size = point1 - point2;
				point1 = point2;
			}
			else
				size = point2 - point1;
			memcpy(child.network.genes + point1, p2.network.genes + point1, size * sizeof(float));
		}
		break;
		case EvolverCrossoverType::Arithmetic:
			for (size_t i = 0; i < child.network.geneCount; i++)
				child.network.genes[i] = (p1.network.genes[i] + p2.network.genes[i]) * 0.5f;
			break;
		case EvolverCrossoverType::ArithmeticProportional:

			float t;
			if (p1.fitness < 0 || p2.fitness < 0)
				t = 0.5f;
			else
				t = p1.fitness / (p1.fitness + p2.fitness);
			for (size_t i = 0; i < child.network.geneCount; i++)
				child.network.genes[i] = p1.network.genes[i] * t + (1 - t) * p2.network.genes[i];
			break;
		case EvolverCrossoverType::Custom:
		{
#ifdef _DEBUG
			if (!crossoverCallback)
				throw std::runtime_error("Crossover callback cannot be nullptr when crossover type is custom");
#endif
			crossoverCallback(child.network.genes, p1, p2, p1.network.genes, p2.network.genes);
		}
		break;
		default:
			throw std::runtime_error("Crossover type is incorrectly defined");
			break;
		}
	}

	void NetworkEvolver::MutateAdd(NetworkOrganism& org)
	{
		uint32_t randomGeneIndex = random.Chance() * org.network.geneCount;
		org.network.genes[randomGeneIndex] = std::clamp(org.network.genes[randomGeneIndex] + mutationScale * random.Normal(), -1.0f, 1.0f);
	}

	void NetworkEvolver::MutateSet(NetworkOrganism& org)
	{
		uint32_t randomGeneIndex = random.ChanceIndex(org.network.geneCount);
		org.network.genes[randomGeneIndex] = random.Value();
	}

	void NetworkEvolver::RunEpisode()
	{
		//if threaded stepping is enabled, the system creates episodeThreadCount threads and uses them to step through the organism
		if (threadedStepping)
		{
			std::vector<std::thread> threads;
			threads.reserve(episodeThreadCount);

			//used to ignore the elite if it is prescient to do so (if elite would have the same fitness as the last episode)
			uint32_t eliteTranslation;
			if (staticEpisodes && currentGeneration != 0)
				eliteTranslation = elitePercent * populationSize;
			else
				eliteTranslation = 0;


			uint32_t perThreadAmount = (populationSize - eliteTranslation) / episodeThreadCount;
			uint32_t extraIndex = episodeThreadCount - ((populationSize - eliteTranslation) % episodeThreadCount);
			uint32_t startIndex = eliteTranslation;
			for (uint32_t t = 0; t < episodeThreadCount; t++)
			{
				//if thread index is more than extraIndex, the thread needs to take care of one more organism
				uint32_t endIndex = startIndex + perThreadAmount + (uint32_t)(t >= extraIndex);
				//just runs the same function as non threaded in parallel 
				//i know for a fact this is not a good way to do threading but I can't find a good example to base my implimentation off of
				//its not like I can keep this thread alive over generations, I have no way to know when the user will call EvaluateGeneration()
				threads.emplace_back(RunEpisodePerThread, this, startIndex, endIndex);
				startIndex = endIndex;
			}

			//rejoin threads
			for (auto& thread : threads)
			{
				thread.join();
			}
		}
		else
		{
			size_t i = 0;
			//if every episode is the same the elite do not need to be stepped through since there fitness will be the same as previous episodes
			if (staticEpisodes && currentGeneration != 0)
				i = elitePercent * populationSize;

			//loop through all organisms and step through them
			for (; i < populationSize; i++)
			{
				//an organism stops stepping if continuestepping evaluates to false or if the step count reaches maxSteps
				for (; organisms[i].steps < maxSteps && organisms[i].continueStepping; organisms[i].steps++)
				{
					//evaluate organism brain
					organisms[i].network.Evaluate(organisms[i].networkInputs, neuralInputSize);
					//call the step callback 
					stepCallback(*this, organisms[i], i);
				}
			}
		}

		if (activateStaticEpisodes)
			staticEpisodes = true;

	}

	void NetworkEvolver::RunEpisodePerThread(NetworkEvolver* obj, uint32_t startIndex, uint32_t endIndex)
	{
		for (size_t i = startIndex; i < endIndex; i++)
		{
			NetworkOrganism& organism = obj->organisms[i];
			//an organism stops stepping if continuestepping evaluates to false or if the step count reaches maxSteps
			for (; organism.steps < obj->maxSteps && organism.continueStepping; organism.steps++)
			{
				//evaluate organism brain
				organism.network.Evaluate(organism.networkInputs, obj->neuralInputSize);
				//call the step callback 
				obj->stepCallback(*obj, organism, i);
			}
		}
	}

	void NetworkEvolver::EvaluateGeneration()
	{
		if (!initialized)
			throw std::runtime_error("NetworkEvolver was not initiated correctly");

		CreateNewGen();
		if (startCallback)
			startCallback(*this, organisms);
		RunEpisode();
		if (endCallback)
			endCallback(*this, organisms);
		currentGeneration++;

	}

	void NetworkEvolver::EvaluateGenerations(uint32_t count)
	{
		if (!initialized)
			throw std::runtime_error("NetworkEvolver was not initiated correctly");
		if (count == 0)
			throw std::runtime_error("Count cannot be 0");

		for (size_t i = 0; i < count; i++)
		{
			if (startCallback)
				startCallback(*this, organisms);
			CreateNewGen();
			RunEpisode();
			currentGeneration++;
			if (endCallback)
				endCallback(*this, organisms);
		}
	}

	const NetworkOrganism& NetworkEvolver::FindBestOrganism() const
	{
		float maxF = organisms[0].fitness;
		uint32_t index = 0;

		for (size_t i = 1; i < populationSize; i++)
		{
			if (organisms[i].fitness > maxF)
			{
				maxF = organisms[i].fitness;
				index = i;
			}
		}
		return organisms[index];
	}

	void NetworkEvolver::SetStaticEpisodes(bool staticEpisodes)
	{
		if (staticEpisodes)
			activateStaticEpisodes = true;
		else
		{
			activateStaticEpisodes = false;
			this->staticEpisodes = false;
		}
	}

	void NetworkEvolver::SetStepCallback(EvolverStepCallback callback)
	{
		if (callback == nullptr)
			throw std::runtime_error("Step callback cannot be set to nullptr");
		stepCallback = callback;
	}

	void NetworkEvolver::SetCustomCrossover(EvolverCustomCrossoverCallback callback)
	{
		crossoverCallback = callback;
		crossoverType = EvolverCrossoverType::Custom;
	}

	void NetworkEvolver::SetCustomMutation(EvolverCustomMutationCallback callback)
	{
		mutationCallback = callback;
		mutationType = EvolverMutationType::Custom;
	}

	void NetworkEvolver::SetCustomSelection(EvolverCustomSelectionCallback callback)
	{
		selectionCallback = callback;
		selectionType = EvolverSelectionType::Custom;

	}

	std::string NetworkEvolver::SavePopulationToString() const
	{
		std::ostringstream ss;

		Save(ss);

		// if failed to load string will be empty
		return ss.str();
	}

	bool NetworkEvolver::LoadPopulationFromFile(std::string filename)
	{
		//open file
		std::ifstream file(filename);
		if (!file.is_open())
			return false;

		bool success = Load(file);

		file.close();
		return success;
	}

	bool NetworkEvolver::LoadPopulationFromString(const std::string& string)
	{
		std::istringstream ss(string);
		if (ss.fail())
			return false;

		bool success = Load(ss);
		return success;
	}

	bool NetworkEvolver::SavePopulationToFile(std::string filename) const
	{
		std::ofstream file(filename);
		if (!file.is_open())
			return false;

		bool success = Save(file);

		file.close();
		return success;
	}

	bool NetworkEvolver::Save(std::ostream& stream) const
	{
		if (!initialized)
			return false;

		//save order:
		// file signature
		// current generation
		// population size
		// random engine
		// network input count
		// network layer count 
		// network layer data 
		// network gene count
		// genes then the fitness value for every organism

		// 8 byte signiture
		// \211 is for the same reason as png 
		// nlve is for nelve evolver
		// 000 is for version 000
		stream << "\211NLVE000";
		stream << currentGeneration;
		stream << populationSize;
		stream << random.engine;
		stream << neuralInputSize;
		const auto& network = organisms[0].network;
		unsigned int layerCount = network.layerCount;
		stream << layerCount;
		for (size_t i = 0; i < layerCount; i++)
		{
			stream << network.layers[i].outputCount;
		}
		unsigned int geneCount = network.geneCount;
		stream << geneCount;
		for (size_t i = 0; i < populationSize; i++)
		{
			for (size_t i = 0; i < geneCount; i++)
			{
				stream << organisms[i].network.genes[i];
			}
			stream << organisms[i].fitness;
		}
		return true;
	}

	bool NetworkEvolver::Load(std::istream& stream)
	{
		//load only loads population data, it does not load other values
		if (!initialized)
			return false;

		//check header is correct
		std::string header(8, ' ');
		std::copy_n(std::istreambuf_iterator<char>(stream.rdbuf()),
			8, std::back_inserter(header));
		if (header != "\211NLVE000")
			return false;

		//delete contents first if already initialized
		Uninitialize();
		
		stream >> currentGeneration;
		stream >> populationSize;
		stream >> random.engine;
		stream >> neuralInputSize;
		int layerCount = 0;
		stream >> layerCount;
		std::vector<int> layers(layerCount - 1);
		uint32_t maxNeurons = 0;
		for (size_t i = 0; i < layerCount - 1; i++)
			stream >> layers[i];
		stream >> neuralOutputSize;

		//create network template
		Network network(neuralInputSize, layers, neuralOutputSize);
		//creat organisms
		organisms = (NetworkOrganism*)(malloc(sizeof(NetworkOrganism) * populationSize));
		for (size_t i = 0; i < populationSize; i++)
		{
			new (organisms + i) NetworkOrganism(network);
			
			const Network& network = organisms[i].GetNetwork();
			for (size_t i = 0; i < network.geneCount; i++)
			{
				stream >> network.genes[i];
			}
			stream >> organisms[i].fitness;
		}

		fitnessOrderedIndexes = new uint32_t[populationSize];
		for (size_t i = 0; i < populationSize; i++)
			fitnessOrderedIndexes[i] = i;

		initialized = true;
		return true;
	}

	void NetworkEvolver::Uninitialize()
	{
		if (initialized)
		{
			for (size_t i = 0; i < populationSize; i++)
				(*(organisms + i)).~NetworkOrganism();
			free(organisms);
			delete[] fitnessOrderedIndexes;
			fitnessOrderedIndexes = nullptr;
			organisms = nullptr;
			initialized = false;
		}
	}
}