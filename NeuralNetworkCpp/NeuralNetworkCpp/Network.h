#pragma once
#include <initializer_list>
#include "NetworkLayer.h"
#include "Maths.h"
#include <stdexcept>
#include <random>

class Network
{
public:
	Network(unsigned int inputs, std::initializer_list<unsigned int> hiddenLayerNeurons, unsigned int outputs);
	~Network();
	Network(const Network& other);
	Network(Network&& other);
	Network& operator=(const Network& other);
	Network& operator=(Network&& other);
	
	/// <summary>
	/// Execute the network and evaluate the inputs
	/// </summary>
	/// <param name="input">The activations of the input layer</param>
	/// <returns>the output activations of the neural network</returns>
	float const* Evaluate(float* input, unsigned int inputCount) const;
	
	void RandomizeValues();
private:
	// componentwise activation function
	float Activate(float weightedInput) const;

	// The layers of the network
	NetworkLayer* layers;
	//Number of layers (not including input layer)
	unsigned int layerCount;
	//the previous value
	
};

