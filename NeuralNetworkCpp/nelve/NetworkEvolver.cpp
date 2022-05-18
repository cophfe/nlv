#include "NetworkEvolver.h"

NetworkEvolver::NetworkEvolver(const NetworkEvolverDefinition& def)
	: population(def.generationSize), maxSteps(def.maxSteps), elitePercent(def.elitePercent), 
	mutationRate(def.mutationRate), stepCallback(def.stepFunction), startCallback(def.startFunction), endCallback(def.endFunction),
	mutationType(def.mutationType), selectionType(def.selectionType), crossoverType(def.crossoverType), currentGeneration(0)
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
	std::vector<unsigned int> fitnessOrderedIndexes(population);
	for (size_t i = 0; i < population; i++)
		fitnessOrderedIndexes[i] = i;
	std::sort(fitnessOrderedIndexes.begin(), fitnessOrderedIndexes.end(), [this](int a, int b) { return organisms[a].fitness > organisms[b].fitness; });

	//create new organism array
	NetworkOrganism* newOrganisms = (NetworkOrganism*)(malloc(sizeof(NetworkOrganism) * population));
	if (newOrganisms == nullptr)
		throw std::runtime_error("Cannot allocate new organism array.");
	memset(newOrganisms, 0, sizeof(NetworkOrganism) * population);

	//Retain elite in next generation
	unsigned int eliteCount = std::min((unsigned int)(elitePercent * population), population);
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
		float inverseTotalFitness = 0;
		//if there are negative fitnesses, add an addition to fitness values to make them all more than 0
		float fitnessAddition = -std::min(organisms[fitnessOrderedIndexes[population - 1]].fitness, 0.0f);

		for (size_t i = 0; i < population; i++)
			inverseTotalFitness += organisms[i].fitness;
		inverseTotalFitness = 1.0f / (inverseTotalFitness + fitnessAddition * population);

		while (childIndex < population)
		{
			NetworkOrganism& p1 = SelectionFitnessProportional(inverseTotalFitness, fitnessAddition);
			NetworkOrganism& p2 = SelectionFitnessProportional(inverseTotalFitness, fitnessAddition);
			new (newOrganisms + childIndex) NetworkOrganism(p1.network);
			Crossover(newOrganisms[childIndex], p1, p2);
			childIndex++;
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
	//stochastic acceptance based 
	unsigned int parentIndex; 
	do {
		parentIndex = random.ChanceIndex(population);
	} while (random.Chance() <= (organisms[parentIndex].fitness + fitnessAddition) * inverseTotalFitness);
	return organisms[parentIndex];
}

void NetworkEvolver::Crossover(NetworkOrganism& child, NetworkOrganism& p1, NetworkOrganism& p2)
{
	switch (crossoverType)
	{
	case EvolverCrossoverType::Uniform:
	{
		for (size_t i = 0; i < child.network.geneCount; i++)
		{
			if (random.Chance() > 0.5f)
				child.network.genes[i] = p2.network.genes[i];
			//otherwise child genes should be equal to p1 genes, which is already true since the child starts as a clone of p1
		}
	}
		break;
	case EvolverCrossoverType::Point:
	{
		unsigned int point = random.ChanceIndex(child.network.geneCount);
		memcpy(child.network.genes, p1.network.genes, point);
		memcpy(child.network.genes + point, p2.network.genes, child.network.geneCount - point);
	}
		break;
	case EvolverCrossoverType::Arithmetic:
		break;
	case EvolverCrossoverType::ArithmeticProportional:
		break;
	case EvolverCrossoverType::Heuristic:
		break;
	default:
		break;
	}
}

void NetworkEvolver::MutateAdd(NetworkOrganism& org)
{
	unsigned int randomGeneIndex = random.Chance() * org.network.geneCount;
	org.network.genes[randomGeneIndex] = std::clamp(org.network.genes[randomGeneIndex] + random.Normal(), -1.0f, 1.0f);
}

void NetworkEvolver::MutateSet(NetworkOrganism& org)
{
	unsigned int randomGeneIndex = random.Chance() * org.network.geneCount;
	org.network.genes[randomGeneIndex] = random.Normal();
}

void NetworkEvolver::StepGen()
{
	
	//loop through all organisms and step through them
	for (size_t i = 0; i < population; i++)
	{
		for (; organisms[i].steps < maxSteps && organisms[i].continueStepping; organisms[i].steps++)
		{
			organisms[i].network.Evaluate(organisms[i].networkInputs, neuralInputSize);
			//call the step callback 
			stepCallback(*this, organisms[i], i);
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

void NetworkEvolver::EvaluateGenerations(unsigned int count)
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
	unsigned int index = 0;

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
