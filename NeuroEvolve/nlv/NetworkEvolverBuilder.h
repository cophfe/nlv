#pragma once
#include "EvolverEnums.h"
#include "Network.h"

namespace nlv
{
	class NetworkEvolver;

	//https://en.wikipedia.org/wiki/Builder_pattern
	struct NetworkEvolverBuilder
	{
		friend NetworkEvolver;
	public:
		/// <param name="networkTemplate">A network that is used as a template for the networks in the evolver</param>
		/// <param name="stepFunction">The step function used in the evolver</param>
		/// <param name="population">The number of individuals in the population</param>
		/// <param name="maxSteps">The maximum number of steps in an episode per individual</param>
		/// <param name="seed">The seed for the initial values of the networks. Setting this to zero automatically assigns a random seed</param>
		NetworkEvolverBuilder(Network& networkTemplate, EvolverStepCallback stepFunction, uint32_t populationSize, uint32_t maxSteps, uint32_t seed = 0);
		/// <param name="startFunction">A callback called before running each episode</param>
		/// <param name="endFunction">A callback called after running each episode</param>
		NetworkEvolverBuilder& SetCallbacks(EvolverGenerationCallback startFunction, EvolverGenerationCallback endFunction);
		/// <param name="elitePercent">The percentage of individuals that are retained every generation</param>
		NetworkEvolverBuilder& SetElitePercent(float elitePercent);
		/// <param name="staticEpisodes">Whether the parameters for each episode change or not</param>
		/// <param name="threadedEpisodes">Whether running episodes is threaded or not</param>
		/// <param name="threadCount">The number of threads used when running episodes</param>
		NetworkEvolverBuilder& SetEpisodeParameters(bool staticEpisodes, bool threadedEpisodes, uint32_t threadCount = 5);
		/// <param name="type">The type of mutation</param>
		/// <param name="mutationRate">The percentage chance a individual is mutated every generation</param>
		/// <param name="mutationScale">The scale of mutation when using EvolverMutationType::Add</param>
		NetworkEvolverBuilder& SetMutation(EvolverMutationType type, float mutationRate, float mutationScale = 1);
		/// <param name="type">The crossover type</param>
		NetworkEvolverBuilder& SetCrossover(EvolverCrossoverType type);
		/// <param name="type">The selection type</param>
		NetworkEvolverBuilder& SetSelection(EvolverSelectionType type);
		/// <summary>
		/// Used to set a custom callback used for selection. Sets selection type to custom.
		/// </summary>
		/// <param name="callback">The callback used for selection</param>
		NetworkEvolverBuilder& SetCustomSelectionType(EvolverCustomSelectionCallback callback);
		/// <summary>
		/// Used to set a custom callback used for mutation. Sets mutation type to custom.
		/// </summary>
		/// <param name="callback">The callback used for mutation</param>
		NetworkEvolverBuilder& SetCustomMutationType(EvolverCustomMutationCallback callback);
		/// <summary>
		/// Used to set a custom callback used for crossover. Sets crossover type to custom.
		/// </summary>
		/// <param name="callback">The callback used for crossover</param>
		NetworkEvolverBuilder& SetCustomCrossoverType(EvolverCustomCrossoverCallback callback);
		/// <summary>
		/// Sets parameters for tournament selection. Sets selection type to tournament.
		/// </summary>
		/// <param name="tournamentSize">The number of organisms competing in a tournament</param>
		NetworkEvolverBuilder& SetTournament(uint32_t tournamentSize);
		/// <param name="ptr">A custom user pointer accessible through the network evolver</param>
		NetworkEvolverBuilder& SetUserPointer(void* ptr);
		/// <summary>
		/// Creates a network evolver from this builder.
		/// </summary>
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