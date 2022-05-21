#include "NetworkEvolver.h"

NetworkEvolver::NetworkEvolver(const NetworkEvolverDefinition& def)
	: population(def.generationSize), maxSteps(def.maxSteps), elitePercent(def.elitePercent),
	mutationRate(def.mutationRate), stepCallback(def.stepFunction), startCallback(def.startFunction), endCallback(def.endFunction),
	mutationType(def.mutationType), selectionType(def.selectionType), crossoverType(def.crossoverType), currentGeneration(0),
	threadedStepping(def.threadedEpisodes), episodeThreadCount(def.episodeThreadCount), staticEpisodes(def.staticEpisodes)
{
	if (population == 0)
		throw std::runtime_error("Generation size cannot be 0");
	if (maxSteps == 0)
		throw std::runtime_error("Max steps cannot be 0");
	if (stepCallback == nullptr)
		throw std::runtime_error("Step callback cannot be nullptr");

	neuralInputSize = def.networkTemplate.GetInputCount();
	neuralOutputSize = def.networkTemplate.GetOutputCount();

	//get an uninitialized array of organisms
	organisms = (NetworkOrganism*)(malloc(sizeof(NetworkOrganism) * population));

	if (def.tournamentSize == 0)
		tournamentSize = std::min(population / 5, 3U);
	else //tournamentSize cannot be less than three
		tournamentSize = std::min(def.tournamentSize, 3U);

	//setup index array used for creating next generations
	fitnessOrderedIndexes = new uint32_t[population];
	for (size_t i = 0; i < population; i++)
		fitnessOrderedIndexes[i] = i;

	if (def.seed == 0)
		random.engine.seed(std::random_device()());
	else
		random.engine.seed(def.seed);
	
	//initiate said array of organisms
	for (size_t i = 0; i < population; i++)
	{
		new (organisms + i) NetworkOrganism(def.networkTemplate);
		organisms[i].network.RandomizeValues(random.engine);
	}
}

NetworkEvolver::~NetworkEvolver()
{
	if (organisms)
	{
		for (size_t i = 0; i < population; i++)
			(*(organisms + i)).~NetworkOrganism();
		free(organisms);
		delete[] fitnessOrderedIndexes;
		fitnessOrderedIndexes = nullptr;
		organisms = nullptr;
	}
	
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
	//Mutation : go over all new children and modify some genes (don't go over the elites? idk thats up to interpretation)

	//Make an index array ordered by the fitnesses of organisms
	std::sort(fitnessOrderedIndexes, fitnessOrderedIndexes + population, [this](int a, int b) { return organisms[a].fitness > organisms[b].fitness; });

	//create new organism array
	NetworkOrganism* newOrganisms = (NetworkOrganism*)(malloc(sizeof(NetworkOrganism) * population));
	if (newOrganisms == nullptr)
		throw std::runtime_error("Cannot allocate new organism array.");
	memset(newOrganisms, 0, sizeof(NetworkOrganism) * population);

	//Retain elite in next generation
	uint32_t eliteCount = std::min((uint32_t)(elitePercent * population), population);
	for (size_t i = 0; i < eliteCount; i++)
	{
		new (newOrganisms + i) NetworkOrganism(organisms[fitnessOrderedIndexes[i]]);
		if (!staticEpisodes)
			newOrganisms[i].Reset();
	}
	
	//the rest of the newGeneration will be populated with children of the previous generation
	int childIndex = eliteCount;

	//DEBUGGGG
	//~~~~~~~~~~~~~~~~~~~~~~~
	std::vector<NetworkOrganism*> parents;
	//~~~~~~~~~~~~~~~~~~~~~~~
	
	//Select parents and crossover to create children
	//note: more than two parents can generate better genomes
	switch (selectionType)
	{
	case EvolverSelectionType::FitnessProportional:
	{
		float inverseTotalFitness = 0;
		//if there are negative fitnesses, add an addition to fitness values to make them all more than 0
		float fitnessAddition = -std::min(organisms[fitnessOrderedIndexes[population - 1]].fitness, 0.0f);

		for (size_t i = 0; i < population; i++)
			inverseTotalFitness += organisms[i].fitness;
		inverseTotalFitness += fitnessAddition * population;
		inverseTotalFitness = 1.0f / (inverseTotalFitness);
		
		while (childIndex < population)
		{
			NetworkOrganism& p1 = SelectionFitnessProportional(inverseTotalFitness, fitnessAddition);
			NetworkOrganism& p2 = SelectionFitnessProportional(inverseTotalFitness, fitnessAddition);

			new (newOrganisms + childIndex) NetworkOrganism(p1.network);
			Crossover(newOrganisms[childIndex], p1, p2);
			childIndex++;

			//DEBUG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			parents.push_back(&p1);
			parents.push_back(&p2);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		}
	}
		break;
	case EvolverSelectionType::StochasticUniversal:
	{
		//https://en.wikipedia.org/wiki/Stochastic_universal_sampling
	
		
	}
	break;
	case EvolverSelectionType::Ranked:
	{
		//1 / guass formula
		float inverseSumOfAllRanks = 2.0f / (population * (population + 1));

		while (childIndex < population)
		{
			NetworkOrganism& p1 = SelectionRanked(inverseSumOfAllRanks);
			NetworkOrganism& p2 = SelectionRanked(inverseSumOfAllRanks);

			new (newOrganisms + childIndex) NetworkOrganism(p1.network);
			Crossover(newOrganisms[childIndex], p1, p2);
			childIndex++;

			//DEBUG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			parents.push_back(&p1);
			parents.push_back(&p2);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		}
	}
		break;
	case EvolverSelectionType::Tournament:
	{
		std::vector<int> tournament(tournamentSize);
		while (childIndex < population)
		{
			//get tournamentSize random indexes
			for (size_t i = 0; i < tournamentSize; i++)
				tournament[i] = random.ChanceIndex(population);

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
			
			//DEBUG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			parents.push_back(p1);
			parents.push_back(p2);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		}
	}
		break;
	case EvolverSelectionType::Boltzman:
	{
		//https://pdfhall.com/chapter-05-the-boltzmann-selection-procedure_5bc2aef2097c47e5298b4598.html
	}
		break;
	default:
		throw std::runtime_error("Selection type is incorrectly defined");
		break;
	}

	//DEBUG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	float averageParentFitness = 0;
	for (size_t i = 0; i < parents.size(); i++)
	{
		averageParentFitness += parents[i]->fitness;
	}
	averageParentFitness /= parents.size();
	std::cout << averageParentFitness << std::endl;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Mutate new children
	switch (mutationType)
	{
	case EvolverMutationType::Set:
		for (size_t i = eliteCount; i < population; i++)
		{
			while(random.Chance() < mutationRate)
				MutateAdd(newOrganisms[i]);
		}
		break;
	case EvolverMutationType::Add:
		for (size_t i = eliteCount; i < population; i++)
		{
			while (random.Chance() < mutationRate)
				MutateSet(newOrganisms[i]);
		}
		break;
	default:
		throw std::runtime_error("Mutation type is incorrectly defined");
		break;
	}
	
	//destroy previous generation
	for (size_t i = 0; i < population; i++)
		(*(organisms + i)).~NetworkOrganism();
	free(organisms);
	
	//set organisms to the new generation
	organisms = newOrganisms;
}

NetworkOrganism& NetworkEvolver::SelectionFitnessProportional(float inverseTotalFitness, float fitnessAddition)
{
	//stochastic acceptance based (faster generally)
	uint32_t parentIndex; 
	float inverseLargestFitness =1/ (organisms[fitnessOrderedIndexes[0]].fitness + fitnessAddition);
	do {
		parentIndex = random.ChanceIndex(population);
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
	//stochastic acceptance based
	uint32_t parentIndex;
	do {
		parentIndex = random.ChanceIndex(population);
	} while (random.Chance() > parentIndex * inverseSumOfAllRanks);
	return organisms[fitnessOrderedIndexes[parentIndex]];

	//float probability = 0;
	//float chance = random.Chance();
	//for (size_t i = 0; i < population; i++)
	//{
	//	probability += parentIndex * inverseSumOfAllRanks;
	//	if (chance <= probability)
	//		return organisms[fitnessOrderedIndexes[i]];
	//}
	////otherwise something went wrong, but this will surely never happen right?
	//return organisms[fitnessOrderedIndexes[population - 1]];

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
		memcpy(child.network.genes + point, p2.network.genes, (child.network.geneCount - point) * sizeof(float));
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
		memcpy(child.network.genes + point1, p2.network.genes, size * sizeof(float));
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
			child.network.genes[i] = p1.network.genes[i] * t + (1 -t) * p2.network.genes[i];
		break;
	case EvolverCrossoverType::Heuristic:
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// UNIMPLEMENTED
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		break;
	default:
		break;
	}
}

void NetworkEvolver::MutateAdd(NetworkOrganism& org)
{
	uint32_t randomGeneIndex = random.Chance() * org.network.geneCount;
	org.network.genes[randomGeneIndex] = std::clamp(org.network.genes[randomGeneIndex] + random.Normal(), -1.0f, 1.0f);
}

void NetworkEvolver::MutateSet(NetworkOrganism& org)
{
	uint32_t randomGeneIndex = random.Chance() * org.network.geneCount;
	org.network.genes[randomGeneIndex] = random.Normal();
}

void NetworkEvolver::RunEpisode()
{
	//if threaded stepping is enabled, the system creates 10 threads and uses them to step through the organism
	//haha note: threaded is way slower than non threaded, making threads like this is super super slow plus they have to clone the cache if they actually want to be on a different core
	// plus probably some other stuff
	if (threadedStepping)
	{
		std::vector<std::thread> threads;
		threads.reserve(episodeThreadCount);

		//used to ignore the elite if it is prescient to do so (if elite would have the same fitness as the last episode)
		uint32_t eliteTranslation;
		if (staticEpisodes && currentGeneration != 0)
			eliteTranslation = elitePercent * population;
		else
			eliteTranslation = 0;

		
		uint32_t perThreadAmount = (population - eliteTranslation) / episodeThreadCount;
		uint32_t extraIndex = episodeThreadCount- ((population - eliteTranslation) % episodeThreadCount);
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
			i = elitePercent * population;

		//loop through all organisms and step through them
		for (; i < population; i++)
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
	if (startCallback)
		startCallback(*this, organisms);
	CreateNewGen();
	RunEpisode();
	currentGeneration++;
	if (endCallback)
		endCallback(*this, organisms);

}

void NetworkEvolver::EvaluateGenerations(uint32_t count)
{
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

	for (size_t i = 1; i < population; i++)
	{
		if (organisms[i].fitness > maxF)
		{
			maxF = organisms[i].fitness;
			index = i;
		}
	}
	return organisms[index];
}

float NetworkEvolver::FindFitnessRange() const
{
	float minF = organisms[0].fitness;
	float maxF = organisms[0].fitness;

	for (size_t i = 1; i < population; i++)
	{
		minF = std::fminf(organisms[i].fitness, minF);
		maxF = std::fmaxf(organisms[i].fitness, maxF);
	}

	return maxF - minF;
}
