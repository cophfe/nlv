#pragma once
#include <initializer_list>
#include "NetworkLayer.h"
#include "Maths.h"
#include <stdexcept>
#include <random>

//a feed forward network implimentation
class Network
{
public:
	Network(unsigned int inputs, std::initializer_list<unsigned int> hiddenLayerNeurons, unsigned int outputs);
	~Network();
	Network(const Network& other);
	Network(Network&& other);
	Network& operator=(const Network& other);
	Network& operator=(Network&& other);
	
	/// <param name="input">The activations of the input layer</param>
	/// <param name="inputCount">The number of neurons in the input layer</param>
	/// <returns>The output activations of the neural network</returns>
	float const* Evaluate(float* input, unsigned int inputCount);

	/// <returns>The last values returned by the evaluate function</returns>
	float const* GetPreviousActivations() const;

	/// <summary>Randomize the network's values</summary>
	void RandomizeValues();
private:
	// componentwise activation function
	float Activate(float weightedInput) const;

	// The layers of the network
	NetworkLayer* layers;
	//Number of layers (not including input layer)
	unsigned int layerCount;
};

