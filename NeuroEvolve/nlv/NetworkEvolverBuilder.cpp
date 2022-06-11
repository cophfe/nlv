#include "NetworkEvolverBuilder.h"
#include "NetworkEvolver.h"

namespace nlv 
{
	NetworkEvolverBuilder::NetworkEvolverBuilder(Network& networkTemplate, EvolverStepCallback stepFunction, uint32_t populationSize, uint32_t maxSteps, uint32_t seed)
		: seed(seed), networkTemplate(networkTemplate), stepFunction(stepFunction), populationSize(populationSize), maxSteps(maxSteps)
	{}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetCallbacks(EvolverGenerationCallback startFunction, EvolverGenerationCallback endFunction)
	{
		this->startFunction = startFunction;
		this->endFunction = endFunction;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetElitePercent(float elitePercent)
	{
		this->elitePercent = std::clamp(elitePercent, 0.0f, 1.0f);
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetEpisodeParameters(bool staticEpisodes, bool threadedEpisodes, uint32_t threadCount)
	{
		this->staticEpisodes = staticEpisodes;
		this->threadedEpisodes = threadedEpisodes;
		this->episodeThreadCount = threadCount;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetMutation(EvolverMutationType type, float mutationRate, float mutationScale)
	{
		mutationType = type;
		this->mutationRate = std::clamp(mutationRate, 0.0f, 1.0f);
		this->mutationScale = mutationScale;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetCrossover(EvolverCrossoverType type)
	{
		crossoverType = type;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetSelection(EvolverSelectionType type)
	{
		selectionType = type;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetCustomSelectionType(EvolverCustomSelectionCallback callback)
	{
		selectionType = EvolverSelectionType::Custom;
		selectionCallback = callback;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetCustomMutationType(EvolverCustomMutationCallback callback)
	{
		mutationType = EvolverMutationType::Custom;
		mutationCallback = callback;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetCustomCrossoverType(EvolverCustomCrossoverCallback callback)
	{
		crossoverType = EvolverCrossoverType::Custom;
		crossoverCallback = callback;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetTournament(uint32_t tournamentSize)
	{
		selectionType = EvolverSelectionType::Tournament;
		this->tournamentSize = tournamentSize;
		return *this;
	}

	NetworkEvolverBuilder& NetworkEvolverBuilder::SetUserPointer(void* ptr)
	{
		userPtr = ptr;
		return *this;
	}

	NetworkEvolver NetworkEvolverBuilder::Build()
	{
		return NetworkEvolver(*this);
	}
}