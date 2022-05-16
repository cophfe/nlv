#include "NetworkEvolver.h"

NetworkEvolver::NetworkEvolver(const NetworkEvolverDefinition& def)
	: generationSize(def.generationSize), maxSteps(def.maxSteps), elitePercent(def.elitePercent), 
	mutationRate(def.mutationRate), stepCallback(def.stepFunction), startCallback(def.startFunction), endCallback(def.endFunction), threadedStepping(def.threadedStepping),
	mutationType(def.mutationType), selectionType(def.selectionType), crossoverType(def.crossoverType), currentGeneration(0)
{
	if (generationSize == 0)
		throw std::runtime_error("Generation size cannot be 0");
	if (maxSteps == 0)
		throw std::runtime_error("Max steps cannot be 0");
	if (stepCallback == nullptr)
		throw std::runtime_error("Step callback cannot be nullptr");
		
	neuralInputSize = def.networkTemplate.GetInputCount();
	neuralOutputSize = def.networkTemplate.GetOutputCount();

	//get an uninitialized array of organisms
	organisms = (NetworkOrganism*)(malloc(sizeof(NetworkOrganism) * generationSize));
	//initiate said array of organisms
	std::random_device rand;
	std::default_random_engine rEngine(rand());

	for (size_t i = 0; i < generationSize; i++)
	{
		new (organisms + i) NetworkOrganism(def.networkTemplate);
		organisms[i].network.RandomizeValues(rEngine);
	}
}

NetworkEvolver::~NetworkEvolver()
{
	if (organisms)
	{
		for (size_t i = 0; i < generationSize; i++)
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
	//Selection : select two parents
	//Crossover : crossover said parents to create a child
	//Loop: do selection and crossover until you have enough in the generation
	//Mutation : go over all new children and modify some genes (don't go over the elites? idk thats up to interpretation)

	//Make an index array ordered by the fitnesses of organisms
	std::vector<unsigned int> fitnessOrderedIndexes(generationSize);
	for (size_t i = 0; i < generationSize; i++)
		fitnessOrderedIndexes[i] = i;
	std::sort(fitnessOrderedIndexes.begin(), fitnessOrderedIndexes.end(), [this](int a, int b) { return organisms[a].fitness < organisms[b].fitness; });
	for (size_t i = 0; i < generationSize; i++)
		std::cout << i << ": " << organisms[fitnessOrderedIndexes[i]].fitness << std::endl;

	//create new organism array
	NetworkOrganism* newOrganisms = (NetworkOrganism*)(malloc(sizeof(NetworkOrganism) * generationSize));
	if (newOrganisms == nullptr)
		throw std::runtime_error("Cannot allocate new organism array.");

	//copy elite over (should work as long as destructors aren't called in previous elites)
	unsigned int eliteCount = std::max((unsigned int)(elitePercent * generationSize), generationSize);
	memcpy(newOrganisms, organisms, eliteCount * sizeof(NetworkOrganism));
	
	//the rest of the newGeneration will be populated with children of the previous generation
	int childIndex = eliteCount;

	//Select parents and crossover to create children
	//note: more than two parents can generate better genomes
	switch (selectionType)
	{
	case EvolverSelectionType::FitnessProportional:
	{

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

	}
		break;
	default:
		throw std::runtime_error("Selection type is incorrectly defined");
		break;
	}

	//Mutate new children
	switch (mutationType)
	{
	case EvolverMutationType::Randomized:
		for (size_t i = eliteCount; i < generationSize; i++)
		{
			Network& net = newOrganisms[i].network;
			for (size_t i = 0; i < net.layerCount; i++)
			{
				NetworkLayer& layer = net.layers[i];
				for (size_t i = 0; i < layer.outputCount; i++)
				{
					if (randomVal < mutationRate)
					{
						layer.biases[i] = otherRandomVal;
					}
				}
				for (size_t i = 0; i < layer.outputCount * layer.inputCount; i++)
				{
					if (randomVal < mutationRate)
					{
						layer.weights[i] = otherRandomVal;
					}
				}
			}
		}
		break;
	case EvolverMutationType::Guassan:
		for (size_t i = eliteCount; i < generationSize; i++)
		{
			Network& net = newOrganisms[i].network;
			for (size_t i = 0; i < net.layerCount; i++)
			{
				NetworkLayer& layer = net.layers[i];
				for (size_t i = 0; i < layer.outputCount; i++)
				{
					if (randomVal < mutationRate)
					{
						layer.biases[i] = std::clamp(layer.biases[i] + normallyDistributedVal, -1, 1);
					}
				}
				for (size_t i = 0; i < layer.outputCount * layer.inputCount; i++)
				{
					if (randomVal < mutationRate)
					{
						layer.biases[i] = std::clamp(layer.biases[i] + normallyDistributedVal, -1, 1);
					}
				}
			}
		}
		break;
	default:
		throw std::runtime_error("Mutation type is incorrectly defined");
		break;
	}
	
	//destroy previous generation
	//don't destruct the elite (thanks free() for allowing this)
	for (size_t i = eliteCount; i < generationSize; i++)
		(*(organisms + i)).~NetworkOrganism();
	free(organisms);
	
	//set organisms to the new generation
	organisms = newOrganisms;
}

void NetworkEvolver::StepGen()
{
	if (threadedStepping)
	{
		//make a thread for each organism and step through them at the same time
		//this could potentially make thousands of threads. its definitely better to have less threads doing a share of organisms each
		//should also push the creation of this into runGeneration/runGenerations, making it possible to keep threads running over multiple generations
		std::thread* threads = new std::thread[generationSize];
		for (size_t i = 0; i < generationSize; i++)
		{
			threads[i] = std::thread([i, this]()
			{
				//step through each organism
				while (organisms[i].steps < maxSteps)
				{
					organisms[i].network.Evaluate(organisms[i].networkInputs, neuralInputSize);
					stepCallback(*this, organisms[i], i);
					if (!organisms[i].continueStepping)
						return;
					organisms[i].steps++;
				}
			});
		}
		for (size_t i = 0; i < generationSize; i++)
		{
			threads[i].join();
		}
	}
	else
	{
		for (size_t i = 0; i < maxSteps; i++)
		{
			bool allDone = true;

			//loop through all organisms and step through them
			for (size_t i = 0; i < generationSize; i++)
			{
				if (organisms[i].steps >= maxSteps || !organisms[i].continueStepping)
					continue;
				organisms[i].network.Evaluate(organisms[i].networkInputs, neuralInputSize);
				//call the step callback
				stepCallback(*this, organisms[i], i);
				organisms[i].steps++;
				allDone = false;
			}
			if (allDone)
				break;
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

	for (size_t i = 1; i < generationSize; i++)
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

	for (size_t i = 1; i < generationSize; i++)
	{
		minF = std::fminf(organisms[i].fitness, minF);
		maxF = std::fmaxf(organisms[i].fitness, maxF);
	}

	return maxF - minF;
}
