#pragma once
#include "EvolverEnums.h"
#include "Network.h"

namespace nlv
{
	class NetworkEvolver;

	struct NetworkEvolverBuilder
	{
		friend NetworkEvolver;
		// networkTemplate: A network that is used as a template for the networks in the evolver
		// stepFunction: The step function used in the evolver
		// population: The number of individuals in the population
		// maxSteps: The maximum number of steps in an episode per individual
		// seed: The seed for the initial values of the networks. Setting this to zero automatically assigns a random seed
		NetworkEvolverBuilder(Network& networkTemplate, EvolverStepCallback stepFunction, uint32_t populationSize, uint32_t maxSteps, uint32_t seed = 0);
		// startFunction: A callback called before running each episode
		// endFunction: A callback called after running each episode
		NetworkEvolverBuilder& SetCallbacks(EvolverGenerationCallback startFunction, EvolverGenerationCallback endFunction);
		// elitePercent: The percentage of individuals that are retained every generation
		NetworkEvolverBuilder& SetElitePercent(float elitePercent);
		// staticEpisodes: Whether the parameters for each episode change or not
		// threadedEpisodes: Whether running episodes is threaded or not
		// threadCount: The number of threads used when running episodes
		NetworkEvolverBuilder& SetEpisodeParameters(bool staticEpisodes, bool threadedEpisodes, uint32_t threadCount = 5);
		// type: The type of mutation
		// mutationRate: The percentage chance a individual is mutated every generation
		// mutationScale: The scale of mutation when using EvolverMutationType::Add
		NetworkEvolverBuilder& SetMutation(EvolverMutationType type, float mutationRate, float mutationScale = 1);
		// type: The crossover type
		NetworkEvolverBuilder& SetCrossover(EvolverCrossoverType type);
		// The selection type
		NetworkEvolverBuilder& SetSelection(EvolverSelectionType type);
		// Used to set a custom callback used for selection. Sets selection type to custom.
		// callback: The callback used for selection
		NetworkEvolverBuilder& SetCustomSelectionType(EvolverCustomSelectionCallback callback);
		// Used to set a custom callback used for mutation. Sets mutation type to custom.
		// callback: The callback used for mutation
		NetworkEvolverBuilder& SetCustomMutationType(EvolverCustomMutationCallback callback);
		// Used to set a custom callback used for crossover. Sets crossover type to custom.
		// callback: The callback used for crossover
		NetworkEvolverBuilder& SetCustomCrossoverType(EvolverCustomCrossoverCallback callback);
		// Sets parameters for tournament selection. Sets selection type to tournament.
		// tournamentSize: The number of organisms competing in a tournament
		NetworkEvolverBuilder& SetTournament(uint32_t tournamentSize);
		// ptr: A custom user pointer accessible through the network evolver
		NetworkEvolverBuilder& SetUserPointer(void* ptr);
		// Creates a network evolver from builder parameters.
		NetworkEvolver Build();

	private:
		Network& networkTemplate;
		EvolverStepCallback stepFunction;
		EvolverGenerationCallback startFunction = nullptr;
		EvolverGenerationCallback endFunction = nullptr;
		EvolverCustomSelectionCallback selectionCallback = nullptr; //for selectiontype::custom
		EvolverCustomCrossoverCallback crossoverCallback = nullptr; //for crossovertype::custom
		EvolverCustomMutationCallback mutationCallback = nullptr; //for mutationtype::custom
		void* userPtr = nullptr;
		uint32_t populationSize;
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
}