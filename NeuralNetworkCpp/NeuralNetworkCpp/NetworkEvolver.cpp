#include "NetworkEvolver.h"

void NetworkEvolver::CreateNextGeneration()
{
	//here is the hard part

}

void NetworkEvolver::StepGeneration()
{
	if (threadedStepping)
	{
		//make a thread for each organism and step through them at the same time
		//this could potentially make thousands of threads. its probably not a good idea
		std::thread* threads = new std::thread[generationSize];
		for (size_t i = 0; i < generationSize; i++)
		{
			threads[i] = std::thread([i, this]()
			{
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
	//aaand this functions work is done.
}

void NetworkEvolver::RunGeneration()
{
	CreateNextGeneration();
	StepGeneration();
}
