#pragma once
#include "Network.h"
#include "NetworkOrganism.h"
#include <thread>
#include "Maths.h"
#include <iostream>
#include <random>
#include <algorithm>
#include "EvolverEnums.h"
#include "EvolverBuilder.h"

//This evolver works best with problems that can be answered using a markov chain
//if a descision can not be made exclusively using the previous state, it will probably not work great
//if the user inputs data from previous states into the network, this isn't necessarily true 
class NetworkEvolver
{
private:
	friend EvolverBuilder;
	// Create network evolver from a network evolver definition
	NetworkEvolver(const EvolverBuilder& def);
public:
	~NetworkEvolver();
	//yes I am lazy, im so happy you noticed <3
	NetworkEvolver(const NetworkEvolver& other) = delete;
	NetworkEvolver& operator=(const NetworkEvolver& other) = delete;
	// Load data
	//////////////////////////////////////static NetworkEvolver LoadFromFile(std::string file);
	//////////////////////////////////////static std::string LoadToString(std::string file);
	// Save data
	//////////////////////////////////////void SaveToFile(std::string file);
	//////////////////////////////////////std::string SaveToString();
	// Create the new generation and run an episode to determine fitness values
	void EvaluateGeneration();
	void EvaluateGenerations(uint32_t count);
	// Finds the best performing organism in the last generation
	const NetworkOrganism& FindBestOrganism() const;
	// Calculates the range of fitnesses in the last generation
	float FindFitnessRange() const;
	// Returns the array containing the last generation of organisms
	inline const NetworkOrganism const* GetOrganisms() const { return organisms; }
	inline uint32_t GetGeneration() const { return currentGeneration; }
	// Get the amount of organisms in a generation
	inline uint32_t GetPopulation() const { return population; }
	// Return the user defined pointer
	inline void* GetUserPointer() const { return userPointer; }
	// Returns if stepping is threaded or not
	inline bool GetThreadedStepping() { return threadedStepping; }
	inline EvolverStepCallback GetStepCallback() { return stepCallback; }
	inline EvolverGenerationCallback GetStartCallback() { return startCallback; }
	inline EvolverGenerationCallback GetEndCallback() { return endCallback; }
	inline EvolverCrossoverType GetCrossoverType() { return crossoverType; }
	inline EvolverMutationType GetMutationType() { return mutationType; }
	inline EvolverSelectionType GetSelectionType() { return selectionType; }
	inline bool GetStaticEpisodes() { return staticEpisodes; }
	inline uint32_t GetTournamentSize() { return tournamentSize; }

	inline void SetUserPointer(void* ptr) { userPointer = ptr; }
	inline void SetMutationRate(float rate) { mutationRate = rate; }
	inline void SetElitePercent(float percent) { elitePercent = percent; }
	inline void SetMaxSteps(uint32_t max) { maxSteps = max; }
	inline void SetThreadedStepping(bool threaded) { threadedStepping = threaded; }
	inline void SetStepCallback (EvolverStepCallback callback) { stepCallback = callback; }
	inline void SetStartCallback (EvolverGenerationCallback callback) { startCallback = callback; }
	inline void SetEndCallback (EvolverGenerationCallback callback) { endCallback = callback; }
	inline void SetCrossoverType (EvolverCrossoverType type) { crossoverType = type; }
	inline void SetMutationType (EvolverMutationType type) { mutationType = type; }
	inline void SetSelectionType (EvolverSelectionType type) { selectionType = type; }
	inline void SetStaticEpisodes (bool staticEpisodes) { this->staticEpisodes = staticEpisodes; }
	inline void SetTournamentSize(uint32_t size) { tournamentSize = std::max(3U, size); }

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

	struct EvolverRandom {
		std::default_random_engine engine;
		std::uniform_real_distribution<float> dist = std::uniform_real_distribution<float>(-1.0f, 1.0f);
		std::normal_distribution<float> guassan = std::normal_distribution<float>(0, 1.0f);

		//between -1 and 1
		inline float Value() { return dist(engine); }
		//between 0 and 1
		inline float Chance() { return (dist(engine) + 1.0f) * 0.5f; }
		//value on normal distribution
		inline float Normal() { return guassan(engine); }
		inline uint32_t ChanceIndex(uint32_t size) { return Chance() * (size - 1); }
	} random;
	//litteraly an array of ints used to index into the organisms array
	uint32_t* fitnessOrderedIndexes;
	// Called for each organism for every step. Allows the user to modify values used for the organism's next step
	// Inside this callback no values accessed by other organisms should be modified.
	EvolverStepCallback stepCallback;
	//Called once at the start of a generation. Allows the user to setup initial input values for the organism, as well as any variables used on the users side.
	//Neither this or endCallback need to be set.
	EvolverGenerationCallback startCallback;
	//Called once at the end of a generation. For whatever the user needs.
	EvolverGenerationCallback endCallback;
	//Custom callbacks
	EvolverCustomSelectionCallback selectionCallback;
	EvolverCustomCrossoverCallback crossoverCallback;
	EvolverCustomMutationCallback mutationCallback;
	//User pointer, pointing to whatever they want it to point to
	void* userPointer;
	// the organisms in the current generation. Not accessible outside of the evolver.
	NetworkOrganism* organisms;
	// The number of organisms in a given generation
	uint32_t population;
	// The size of the neural networks' input and output arrays
	uint32_t neuralInputSize, neuralOutputSize;
	// The percentage chance that a gene is mutated
	float mutationRate;
	// The scale of change in a mutated gene when using MutationType::Add
	float mutationScale;
	// The percentage of individuals from one generation who are directly cloned to the next generation
	float elitePercent;
	// The maximum number of steps an organism can take
	uint32_t maxSteps;
	// The current generation index
	uint32_t currentGeneration;
	//The type of mutation used
	EvolverMutationType mutationType;
	//The type of crossover used
	EvolverCrossoverType crossoverType;
	//The type of selection used
	EvolverSelectionType selectionType;
	//Whether stepping through organisms is threaded or not.
	bool threadedStepping;
	//Whether every episode is the same as the last
	bool staticEpisodes;
	//The number of threads created
	uint32_t episodeThreadCount;
	//the size of the tournament if using tournament selection
	uint32_t tournamentSize;
};

