#pragma once
#include "Network.h"
#include "NetworkOrganism.h"
#include <thread>
#include "Maths.h"
#include <iostream>

typedef void(*EvolverStepCallback)(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex);
typedef void(*EvolverGenerationCallback)(const NetworkEvolver& evolver, NetworkOrganism* organisms);

// Note: in this class a gene is considered to be a weight or a bias in a neural network
// When a gene is mutated, this decides how exactly that will take place
enum class EvolverMutationType : char
{
	// The gene is set to a random value between -1 and 1
	Randomized,
	// The gene has a random value from a normal distribution added to it (mean 0, standard deviation 1, clamped between -1 and 1)
	Guassan
};

// How the parents used for crossover are selected
enum class EvolverSelectionType : char
{
	// Parents are selected based on the percentage of total fitness they have
	FitnessProportional,
	// Parents are selected based on their fitness rank
	Ranked,
	// A tournament is performed between all in the population. 
	Tournament,
	// https://pdfhall.com/chapter-05-the-boltzmann-selection-procedure_5bc2aef2097c47e5298b4598.html
	Boltzman
};

// How two parents from the previous generation are crossed over to create a child network
enum class EvolverCrossoverType : char
{
	// Chooses which parent a gene is taken from with a 50% chance for either parent
	Uniform,
	// Chooses which parent a gene is taken from using a chance based on the proportion of fitness values between parents
	UniformProportional,
	// Linearly combines the genes from the two parent genomes
	Arithmetic,
	// Linearly combines the genes from the two parent genomes, interpolated based on the proportion of fitness values between them
	ArithmeticProportional,
	// idk what this one is but it sounds cool
	Heuristic
	//[no one point or two point crossover because there is no *good* 
	//	way to represent the network as a 1D array without making weird ambiguous descisions]
};

struct NetworkEvolverDefinition
{
public:
	NetworkEvolverDefinition() = default;
	NetworkEvolverDefinition(Network& networkTemplate, unsigned int generationSize, unsigned int maxSteps,
		float elitePercent, float mutationRate, EvolverStepCallback stepFunction, EvolverMutationType mutationType, EvolverCrossoverType crossoverType,
		EvolverSelectionType selectionType, bool threadedStepping, EvolverGenerationCallback startFunction,
		EvolverGenerationCallback endFunction)
		: networkTemplate(networkTemplate), generationSize(generationSize), maxSteps(maxSteps), elitePercent(elitePercent),
		mutationRate(mutationRate), stepFunction(stepFunction), startFunction(startFunction), endFunction(endFunction),
		mutationType(mutationType), crossoverType(crossoverType), selectionType(selectionType), threadedStepping(threadedStepping)
	{}

	Network& networkTemplate;
	EvolverStepCallback stepFunction;
	EvolverGenerationCallback startFunction;
	EvolverGenerationCallback endFunction;
	unsigned int generationSize;
	unsigned int maxSteps;
	float elitePercent;
	float mutationRate;
	EvolverMutationType mutationType;
	EvolverCrossoverType crossoverType;
	EvolverSelectionType selectionType;
	bool threadedStepping;
};


//This evolver will create a markov chain
//if a descision can not be made exclusively using the previous state, it will probably not work great
//if the user inputs the previous states into the network, this isn't necessarily true.
class NetworkEvolver
{
public:
	// Create network evolver from a network evolver definition
	NetworkEvolver(const NetworkEvolverDefinition& def);
	~NetworkEvolver();
	NetworkEvolver(const NetworkEvolver& other) = delete;
	NetworkEvolver& operator=(const NetworkEvolver& other) = delete;
	// Load data
	//////////////////////////////////////static NetworkEvolver LoadFromFile(std::string file);
	//////////////////////////////////////static std::string LoadToString(std::string file);
	// Save data
	//////////////////////////////////////void SaveToFile(std::string file);
	//////////////////////////////////////std::string SaveToString();
	// Create the new generation, step through
	void EvaluateGeneration();
	void EvaluateGenerations(unsigned int count);
	// Finds the best performing organism in the last generation
	const NetworkOrganism& FindBestOrganism() const;
	// Calculates the range of fitnesses in the last generation
	float FindFitnessRange() const;
	// Returns the array containing the last generation of organisms
	inline const NetworkOrganism const* GetGeneration() const { return organisms; };
	// Get the amount of organisms in a generation
	inline unsigned int GetGenerationSize() const { return generationSize; };

private:
	// Create the next generation based on values from the last generation
	void CreateNewGen();
	// Step through the current generation
	void StepGen();

	// Called for each organism for every step. Allows the user to modify values used for the organism's next step
	// Inside this callback no values accessed by other organisms should be modified.
	EvolverStepCallback stepCallback;
	//Called once at the start of a generation. Allows the user to setup initial input values for the organism, as well as any variables used on the users side.
	//Neither this or endCallback need to be set.
	EvolverGenerationCallback startCallback;
	//Called once at the end of a generation. For whatever the user needs.
	EvolverGenerationCallback endCallback;
	// the organisms in the current generation. Not accessible outside of the evolver.
	NetworkOrganism* organisms;
	// The number of organisms in a given generation
	unsigned int generationSize;
	// The size of the neural networks' input and output arrays
	unsigned int neuralInputSize, neuralOutputSize;
	// The percentage chance that a gene is mutated
	float mutationRate;
	// The percentage of individuals from one generation who are directly cloned to the next generation
	float elitePercent;
	// The maximum number of steps an organism can take
	unsigned int maxSteps;
	// The current generation index
	unsigned int currentGeneration;
	// Whether stepping is threaded or not. Disabling this will severely impact performance
	// Note: mutation is still threaded
	bool threadedStepping;
	//The type of mutation used
	EvolverMutationType mutationType;
	//The type of crossover used
	EvolverCrossoverType crossoverType;
	//The type of selection used
	EvolverSelectionType selectionType;
};

