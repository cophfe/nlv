#pragma once
#include "Network.h"
#include "NetworkGenome.h"
#include <thread>

typedef void(*EvolverStepCallback)(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex);

// Note: in this library the 
// When a gene is mutated, this decides how exactly that will take place
enum class EvolverMutationType : char
{
	// The gene is set to a random value between -1 and 1
	Randomized,
	// The gene has a random value from a normal distribution (mean 0, standard deviation 1) added to it (clamped between -1 and 1)
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
	Network& networkTemplate;
	unsigned int generationSize;
	unsigned int maxSteps;
	float elitePercent;
	float mutationRate;
	EvolverStepCallback stepFunction;
	EvolverMutationType mutationType;
	EvolverCrossoverType crossoverType;
	EvolverSelectionType selectionType;
	bool threadedStepping;
};

class NetworkEvolver
{
public:
	// Create network evolver from a network evolver definition
	NetworkEvolver(const NetworkEvolverDefinition& def);
	// Load data
	static NetworkEvolver LoadFromFile(std::string file);
	static std::string LoadToString(std::string file);
	// Save data
	void SaveToFile(std::string file);
	std::string SaveToString();
	// Create the new generation, step through
	void RunGeneration();
	// Finds the best performing organism in the last generation
	const NetworkOrganism& FindBestOrganism() const;
	// Returns the array containing the last generation of organisms
	const NetworkOrganism const* GetGeneration() const;
	// Get the amount of organisms in a generation
	unsigned int GetGenerationSize() const;

private:
	// Create the next generation based on values from the last generation
	void CreateNextGeneration();
	// Step through the current generation
	void StepGeneration();

	// Called for each organism for every step. Allows the user to modify values used for the organism's next step
	// Inside this callback no values accessed by other organisms should be modified.
	EvolverStepCallback stepCallback;
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

