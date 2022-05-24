#pragma once
#include "EvolverEnums.h"
#include "Network.h"
class NetworkEvolver;

//https://en.wikipedia.org/wiki/Builder_pattern
struct EvolverBuilder
{
public:
	EvolverBuilder(Network& networkTemplate, EvolverStepCallback stepFunction, uint32_t population, uint32_t maxSteps, uint32_t seed);
	EvolverBuilder& SetCallbacks(EvolverGenerationCallback startFunction, EvolverGenerationCallback endFunction);
	EvolverBuilder& SetElitePercent(float elitePercent);
	EvolverBuilder& SetEpisodeParameters(bool staticEpisodes, bool threadedEpisodes, uint32_t threadCount = 5);
	EvolverBuilder& SetMutation(EvolverMutationType type, float mutationRate, float mutationScale = 1);
	EvolverBuilder& SetCrossover(EvolverCrossoverType type);
	EvolverBuilder& SetSelection(EvolverSelectionType type);
	EvolverBuilder& SetCustomSelectionType(EvolverCustomSelectionCallback callback);
	EvolverBuilder& SetCustomMutationType(EvolverCustomMutationCallback callback);
	EvolverBuilder& SetCustomCrossoverType(EvolverCustomCrossoverCallback callback);
	EvolverBuilder& SetTournament(uint32_t tournamentSize);
	EvolverBuilder& SetUserPointer(void* ptr);
	NetworkEvolver&& Build();

	Network& networkTemplate;
	EvolverStepCallback stepFunction;
	EvolverGenerationCallback startFunction = nullptr;
	EvolverGenerationCallback endFunction = nullptr;
	EvolverCustomSelectionCallback selectionCallback = nullptr; //for selectiontype::custom
	EvolverCustomCrossoverCallback crossoverCallback = nullptr; //for crossovertype::custom
	EvolverCustomMutationCallback mutationCallback = nullptr; //for mutationtype::custom
	void* userPtr = nullptr;
	uint32_t generationSize;
	uint32_t maxSteps;
	uint32_t seed;
	float elitePercent = 0;
	float mutationRate = 0.4f;
	float mutationScale = 1.0f; //for mutationtype::add
	uint32_t episodeThreadCount = 0; //for threadedEpisodes == true
	uint32_t tournamentSize = 5; //for selectiontype::tournament
	EvolverMutationType mutationType = EvolverMutationType::Set;
	EvolverCrossoverType crossoverType = EvolverCrossoverType::Uniform;
	EvolverSelectionType selectionType = EvolverSelectionType::Ranked;
	bool threadedEpisodes = false;
	bool staticEpisodes = false;
};