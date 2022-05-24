#pragma once
#include "NetworkOrganism.h"
typedef void(*EvolverStepCallback)(const NetworkEvolver& evolver, NetworkOrganism& organism, int organismIndex);
typedef void(*EvolverGenerationCallback)(const NetworkEvolver& evolver, NetworkOrganism* organisms);
typedef void(*EvolverCustomCrossoverCallback)(float* childGenes, const NetworkOrganism& p1, const NetworkOrganism& p2, float* p1Genes, float* p2Genes);
typedef void(*EvolverCustomMutationCallback)(float* genes, const NetworkOrganism& organism);
typedef NetworkOrganism* (*EvolverCustomSelectionCallback)(NetworkOrganism* organisms, NetworkOrganism*& selectedOrganism);

// Note: in this class a gene is considered to be a weight or a bias in a neural network
// When a gene is mutated, this decides how exactly that will take place
enum class EvolverMutationType : char
{
	// The gene is set to a random value between -1 and 1
	Set,
	// The gene has a random value from a normal distribution added to it (mean 0, standard deviation 1, clamped between -1 and 1)
	Add,
	// Calls a custom mutation function
	Custom
};

// How the parents used for crossover are selected
enum class EvolverSelectionType : char
{
	// Parents are selected based on the percentage of total fitness they have
	FitnessProportional,
	// Parents are selected based on their fitness rank
	Ranked,
	// A tournament is performed on the population based on fitness, the winners are selected. 
	Tournament,
	// A selection type that tries to avoid premature convergance by adapting based on fitness range
	//Boltzman,
	// Calls a custom selection function
	Custom
};

// How two parents from the previous generation are crossed over to create a child network
enum class EvolverCrossoverType : char
{
	// Chooses which parent a gene is taken from with a 50% chance for either parent
	Uniform,
	// chooses a random point on the genome. one side's genes will be taken from p1, the other p2
	Point,
	// chooses 2 random points on the genome. The inbetween genes will be taken from p2, the other genes will be taken from p1
	TwoPoint,
	// Linearly combines the genes from the two parent genomes
	Arithmetic,
	// Linearly combines the genes from the two parent genomes, interpolated based on the proportion of fitness values between them
	// if either of the fitness values are negative, this will revert to regular arithmetic crossover.
	ArithmeticProportional,
	// Calls a custom crossover function
	Custom
};