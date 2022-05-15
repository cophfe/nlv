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
		
	//get an uninitialized array of organisms
	organisms = (NetworkOrganism*)(::operator new(sizeof(NetworkOrganism) * generationSize));
	//initiate said array of organisms
	for (size_t i = 0; i < generationSize; i++)
	{
		new (organisms + i) NetworkOrganism(def.networkTemplate);
		organisms[i].network.RandomizeValues();
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
	//Selection
	//Crossover
	//Mutation
	
	switch (selectionType)
	{
	case EvolverSelectionType::FitnessProportional:

		break;
	case EvolverSelectionType::Ranked:
		
		break;
	case EvolverSelectionType::Tournament:
		
		break;
	case EvolverSelectionType::Boltzman:
		
		break;
	default:
		break;
	}
	//Crossover & mutation (loop until done)

	currentGeneration++;
}

void NetworkEvolver::StepGen()
{
	if (threadedStepping)
	{
		//make a thread for each organism and step through them at the same time
		//this could potentially make thousands of threads. its probably not a good idea :\
		//note: should push the creation of this into runGeneration, allowing me to keep threads running over multiple generations
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
