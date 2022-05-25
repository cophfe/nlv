#include "EvolverBuilder.h"
#include "NetworkEvolver.h"

EvolverBuilder::EvolverBuilder(Network& networkTemplate, EvolverStepCallback stepFunction, uint32_t populationSize, uint32_t maxSteps, uint32_t seed)
	: seed(seed), networkTemplate(networkTemplate), stepFunction(stepFunction), populationSize(populationSize), maxSteps(maxSteps)
{}

EvolverBuilder& EvolverBuilder::SetCallbacks(EvolverGenerationCallback startFunction, EvolverGenerationCallback endFunction)
{
	this->startFunction = startFunction;
	this->endFunction = endFunction;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetElitePercent(float elitePercent)
{
	this->elitePercent = std::clamp(elitePercent, 0.0f, 1.0f);
	return *this;
}

EvolverBuilder& EvolverBuilder::SetEpisodeParameters(bool staticEpisodes, bool threadedEpisodes, uint32_t threadCount)
{
	this->staticEpisodes = staticEpisodes;
	this->threadedEpisodes = threadedEpisodes;
	this->episodeThreadCount = threadCount;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetMutation(EvolverMutationType type, float mutationRate, float mutationScale)
{
	mutationType = type;
	this->mutationRate = std::clamp(mutationRate, 0.0f, 1.0f);
	this->mutationScale = mutationScale;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetCrossover(EvolverCrossoverType type)
{
	crossoverType = type;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetSelection(EvolverSelectionType type)
{
	selectionType = type;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetCustomSelectionType(EvolverCustomSelectionCallback callback)
{
	selectionType = EvolverSelectionType::Custom;
	selectionCallback = callback;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetCustomMutationType(EvolverCustomMutationCallback callback)
{
	mutationType = EvolverMutationType::Custom;
	mutationCallback = callback;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetCustomCrossoverType(EvolverCustomCrossoverCallback callback)
{
	crossoverType = EvolverCrossoverType::Custom;
	crossoverCallback = callback;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetTournament(uint32_t tournamentSize)
{
	selectionType = EvolverSelectionType::Tournament;
	this->tournamentSize = tournamentSize;
	return *this;
}

EvolverBuilder& EvolverBuilder::SetUserPointer(void* ptr)
{
	userPtr = ptr;
	return *this;
}

NetworkEvolver EvolverBuilder::Build()
{
	return NetworkEvolver(*this);
}
