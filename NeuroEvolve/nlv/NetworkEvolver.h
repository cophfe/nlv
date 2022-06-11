#pragma once
#include "Network.h"
#include "NetworkOrganism.h"
#include <sstream>
#include <random>
#include <algorithm>
#include "EvolverEnums.h"
#include "NetworkEvolverBuilder.h"

namespace nlv
{
	class NetworkEvolver
	{
	public:
		//this will not fully initiate the network evolver
		NetworkEvolver() = default;
		// Create network evolver from a network evolver definition
		NetworkEvolver(const NetworkEvolverBuilder& def);
		// Destructor
		~NetworkEvolver();
		// Move constructor
		NetworkEvolver(NetworkEvolver&& other);
		// Move assignment
		NetworkEvolver& operator=(NetworkEvolver&& other);
		// Copy constructor and assignment are unnesesary, right??
		NetworkEvolver(const NetworkEvolver& other) = delete;
		NetworkEvolver& operator=(const NetworkEvolver& other) = delete;

		// Load data
		bool LoadPopulationFromFile(std::string filename);
		bool LoadPopulationFromString(const std::string& string);

		// Saves population data to a file
		// filename: The filename of which to save the network to
		// Returns whether the save succeeded or failed
		bool SavePopulationToFile(std::string filename) const;

		// Saves population data to a string
		std::string SavePopulationToString() const;

		// Evaluates a generation: constructs a new generation and calculates fitness values for them
		void EvaluateGeneration();
		// Evaluates several generations: constructs a new generation and calculates fitness values for them
		void EvaluateGenerations(uint32_t count);

		//Finds the organism with the highest fitness
		const NetworkOrganism& FindBestOrganism() const;

		//Getters
		inline const NetworkOrganism const* GetPopulationArray() const { return organisms; }
		inline uint32_t GetGeneration() const { return currentGeneration; }
		inline uint32_t GetPopulationSize() const { return populationSize; }
		inline bool GetIfThreadedEpisodes() const { return threadedStepping; }
		inline bool GetStaticEpisodes() const { return staticEpisodes; }
		inline bool GetIsInitiated() const { return initialized; }
		inline float GetMutationScale() const { return mutationScale; }
		inline float GetElitePercent() const { return elitePercent; }
		inline float GetMutationRate() const { return mutationRate; }
		inline uint32_t GetMaxSteps() const { return maxSteps; }
		inline uint32_t GetTournamentSize() const { return tournamentSize; }
		inline uint32_t GetEpisodeThreadCount() const { return episodeThreadCount; }
		inline EvolverStepCallback GetStepCallback() const { return stepCallback; }
		inline EvolverGenerationCallback GetStartCallback() const { return startCallback; }
		inline EvolverGenerationCallback GetEndCallback() const { return endCallback; }
		inline EvolverCrossoverType GetCrossoverType() const { return crossoverType; }
		inline EvolverMutationType GetMutationType() const { return mutationType; }
		inline EvolverSelectionType GetSelectionType() const { return selectionType; }
		inline void* GetUserPointer() const { return userPointer; }
		//Setters
		inline void SetIsThreadedEpisodes(bool threaded) { threadedStepping = threaded; }
		void SetStaticEpisodes(bool staticEpisodes);
		inline void SetMutationRate(float rate) { mutationRate = std::clamp(rate, 0.0f, 1.0f); }
		inline void SetMutationScale(float scale) { mutationScale = scale; }
		inline void SetElitePercent(float percent) { elitePercent = std::clamp(percent, 0.0f, 1.0f); }
		inline void SetMaxSteps(uint32_t max) { maxSteps = std::max(max, 1U); }
		inline void SetTournamentSize(uint32_t size) { tournamentSize = std::max(3U, size); }
		inline void SetEpisodeThreadCount(uint32_t count) { episodeThreadCount = std::max(1U, count); }
		void SetStepCallback(EvolverStepCallback callback);
		inline void SetStartCallback(EvolverGenerationCallback callback) { startCallback = callback; }
		inline void SetEndCallback(EvolverGenerationCallback callback) { endCallback = callback; }
		inline void SetCrossoverType(EvolverCrossoverType type) { crossoverType = type; }
		inline void SetMutationType(EvolverMutationType type) { mutationType = type; }
		inline void SetSelectionType(EvolverSelectionType type) { selectionType = type; }
		inline void SetUserPointer(void* ptr) { userPointer = ptr; }
		void SetCustomCrossover(EvolverCustomCrossoverCallback callback);
		void SetCustomMutation(EvolverCustomMutationCallback callback);
		void SetCustomSelection(EvolverCustomSelectionCallback callback);

	private:
		// Create the next generation based on values from the last generation
		void CreateNewGen();
		//Selection functions
		NetworkOrganism& SelectionFitnessProportional(float fitnessAddition);
		NetworkOrganism& SelectionRanked(float inverseSumOfAllRanks);
		//Crossover function
		void Crossover(NetworkOrganism& child, NetworkOrganism& p1, NetworkOrganism& p2);
		//Mutate functions
		void MutateSet(NetworkOrganism& org);
		void MutateAdd(NetworkOrganism& org);
		// Step through the current generation
		void RunEpisode();
		static void RunEpisodePerThread(NetworkEvolver* obj, uint32_t startIndex, uint32_t endIndex);

		//called by save and load functions to save and load into either string or file streams
		bool Save(std::ostream& stream) const;
		//yeah requires the pointers to be reset by the user because I can't save those
		bool Load(std::istream& stream);

		void Uninitialize();

		struct EvolverRandom {
			//note: mersenne twister is slow
			std::default_random_engine engine;
			//note: these distribiutions mess up the determinism of the evolver because their implimentations change between computers
			std::uniform_real_distribution<float> dist = std::uniform_real_distribution<float>(-1.0f, 1.0f);
			std::normal_distribution<float> guassan = std::normal_distribution<float>(0, 1.0f); // this has a state... godammit.

			//between -1 and 1
			inline float Value() { return dist(engine); }
			//between 0 and 1
			inline float Chance() { return (dist(engine) + 1.0f) * 0.5f; }
			//value on normal distribution
			inline float Normal() { return guassan(engine); }
			inline uint32_t ChanceIndex(uint32_t size) { return Chance() * (size - 1); }
		} random;
		//literaly an array of ints used to index into the organisms array
		uint32_t* fitnessOrderedIndexes = nullptr;
		// Called for each organism for every step. Allows the user to modify values used for the organism's next step
		// Inside this callback no values accessed by other organisms should be modified.
		EvolverStepCallback stepCallback = nullptr;
		//Called once at the start of a generation. Allows the user to setup initial input values for the organism, as well as any variables used on the users side.
		//Neither this or endCallback need to be set.
		EvolverGenerationCallback startCallback = nullptr;
		//Called once at the end of a generation. For whatever the user needs.
		EvolverGenerationCallback endCallback = nullptr;
		//Custom callbacks
		EvolverCustomSelectionCallback selectionCallback = nullptr;
		EvolverCustomCrossoverCallback crossoverCallback = nullptr;
		EvolverCustomMutationCallback mutationCallback = nullptr;
		//User pointer, pointing to whatever they want it to point to
		void* userPointer = nullptr;
		// the organisms in the current generation. Not accessible outside of the evolver.
		NetworkOrganism* organisms = nullptr;
		// The number of organisms in a given generation
		uint32_t populationSize = 0;
		// The size of the neural networks' input and output arrays
		uint32_t neuralInputSize = 0, neuralOutputSize = 0;
		// The percentage chance that a gene is mutated
		float mutationRate = 0;
		// The scale of change in a mutated gene when using MutationType::Add
		float mutationScale = 0;
		// The percentage of individuals from one generation who are directly cloned to the next generation
		float elitePercent = 0;
		// The maximum number of steps an organism can take
		uint32_t maxSteps = 0;
		// The current generation index
		uint32_t currentGeneration = 0;
		//The type of mutation used
		EvolverMutationType mutationType = EvolverMutationType::Add;
		//The type of crossover used
		EvolverCrossoverType crossoverType = EvolverCrossoverType::Arithmetic;
		//The type of selection used
		EvolverSelectionType selectionType = EvolverSelectionType::FitnessProportional;
		//Whether stepping through organisms is threaded or not
		bool threadedStepping = false;
		//Whether every episode is the same as the last
		bool staticEpisodes = false;
		//The number of threads created
		uint32_t episodeThreadCount = 0;
		//the size of the tournament if using tournament selection
		uint32_t tournamentSize = 0;

		//if the evolver is initiated or not. if it has been destroyed or was not constructed correctly this may evaluate to false
		bool initialized = false;

		//for if static episodes is set to true after multiple generations have been run, to prevent issues
		bool activateStaticEpisodes = false;
	};
}

