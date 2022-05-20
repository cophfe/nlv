#include "NetworkEvolver.h"

NetworkEvolver::NetworkEvolver(const NetworkEvolverDefinition& def)
	: population(def.generationSize), maxSteps(def.maxSteps), elitePercent(def.elitePercent), 
	mutationRate(def.mutationRate), stepCallback(def.stepFunction), startCallback(def.startFunction), endCallback(def.endFunction),
	mutationType(def.mutationType), selectionType(def.selectionType), crossoverType(def.crossoverType), currentGeneration(0), threadedStepping(def.threadedStepping)
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
	
	//setup index array used for creating next generations
	fitnessOrderedIndexes = std::vector<uint32_t>(population);
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
	std::sort(fitnessOrderedIndexes.begin(), fitnessOrderedIndexes.end(), [this](int a, int b) { return organisms[a].fitness > organisms[b].fitness; });

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
		float totalFitness = 0;
		//if there are negative fitnesses, add an addition to fitness values to make them all more than 0
		float fitnessAddition = -std::min(organisms[fitnessOrderedIndexes[population - 1]].fitness, 0.0f);

		for (size_t i = 0; i < population; i++)
			totalFitness += organisms[i].fitness;
		totalFitness += fitnessAddition * population;

		//DEBUGGGG
		std::vector<NetworkOrganism*> parents;

		while (childIndex < population)
		{
			float inverseTotalFitness = 1.0f / (totalFitness);
			NetworkOrganism& p1 = SelectionFitnessProportional(inverseTotalFitness, fitnessAddition);
			NetworkOrganism& p2 = SelectionFitnessProportional(inverseTotalFitness, fitnessAddition);

			new (newOrganisms + childIndex) NetworkOrganism(p1.network);
			Crossover(newOrganisms[childIndex], p1, p2);
			childIndex++;

			parents.push_back(&p1);
			parents.push_back(&p2);
		}

		float averageParentFitness = 0;
		for (size_t i = 0; i < parents.size(); i++)
		{
			averageParentFitness += parents[i]->fitness;
		}
		averageParentFitness /= parents.size();
		std::cout << averageParentFitness << std::endl;
	}
		break;
	case EvolverSelectionType::StochasticUniversal:
	{
		//https://en.wikipedia.org/wiki/Stochastic_universal_sampling
	
		
	}
	break;
	case EvolverSelectionType::Ranked:
	{

	}
		break;
	case EvolverSelectionType::Tournament:
	{

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
	//return organisms[fitnessOrderedIndexes[0]];
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

void NetworkEvolver::StepGen()
{
	//if threaded stepping is enabled, the system creates 10 threads and uses them to step through the organism
	//haha note: threaded is way slower than non threaded, making threads like this is super super slow plus they have to clone the cache if they actually want to be on a different core
	// plus probably some other stuff
	if (threadedStepping)
	{
		std::thread threads[steppingThreadCount];
		for (uint32_t i = 0; i < steppingThreadCount; i++)
		{
			//just runs the same function as non threaded in parallel 
			//i know for a fact this is not a good way to do threading but I can't find a good example to base my implimentation off of
			//its not like I can keep this thread alive over generations, I have no way to know when the user will call EvaluateGeneration()
			
			threads[i] = std::thread(
				[this, i]() {
					uint32_t stepAmount = population/steppingThreadCount;
					uint32_t startIndex = i * stepAmount;

					for (size_t j = startIndex; j < startIndex + stepAmount; j++)
					{
						for (; organisms[j].steps < maxSteps && organisms[j].continueStepping; organisms[j].steps++)
						{
							organisms[j].network.Evaluate(organisms[j].networkInputs, neuralInputSize);
							//call the step callback 
							stepCallback(*this, organisms[j], j);
						}
					}
				}
			);
		}

		for (uint32_t i = 0; i < steppingThreadCount; i++)
		{
			threads[i].join();
		}
	}
	else
	{
		//loop through all organisms and step through them
		for (size_t i = 0; i < population; i++)
		{
			//an organism stops stepping if continuestepping evaluates to false or if the step count reaches maxSteps
			for (; organisms[i].steps < maxSteps && organisms[i].continueStepping; organisms[i].steps++)
			{
				organisms[i].network.Evaluate(organisms[i].networkInputs, neuralInputSize);
				//call the step callback 
				stepCallback(*this, organisms[i], i);
			}
		}
	}
	
}

void NetworkEvolver::EvaluateGeneration()
{
	if (startCallback)
		startCallback(*this, organisms);
	CreateNewGen();
	StepGen();
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
		StepGen();
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
