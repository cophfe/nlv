#pragma once
#include <initializer_list>
#include "Maths.h"
#include <stdexcept>
#include <random>

class NetworkEvolver;

//a feed forward network implimentation
class Network
{
	friend NetworkEvolver;
public:
	Network(); //note: this initializes an empty network, which will not be able to do anything
	Network(uint32_t inputs, std::initializer_list<uint32_t> hiddenLayerNeurons, uint32_t outputs);
	~Network();
	Network(const Network& other);
	Network(Network&& other);
	Network& operator=(const Network& other);
	Network& operator=(Network&& other);
	
	/// <param name="input">The activations of the input layer</param>
	/// <param name="inputCount">The number of neurons in the input layer</param>
	/// <returns>The output activations of the neural network</returns>
	float const* Evaluate(float* input, uint32_t inputCount);

	/// <returns>The last values returned by the evaluate function</returns>
	float const* GetPreviousActivations() const;

	/// <returns>The number of input neurons into the network</returns>
	inline uint32_t GetInputCount() const { return inputCount; }
	/// <returns>The number of output neurons from the network</returns>
	inline uint32_t GetOutputCount() const { return layers[layerCount - 1].outputCount; }

	/// <summary>Randomize the network's values</summary>
	void RandomizeValues();
	/// <summary>Randomize the network's values</summary>
	void RandomizeValues(uint32_t seed);
	/// <summary>Randomize the network's values</summary>
	void RandomizeValues(std::default_random_engine& randEngine);

	float GetWeight(uint32_t layer, uint32_t currentNeuronIndex, uint32_t lastNeuronIndex) const;
	void SetWeight(uint32_t layer, uint32_t currentNeuronIndex, uint32_t lastNeuronIndex, float value);
	float GetBias(uint32_t layer, uint32_t neuronIndex) const;
	void SetBias(uint32_t layer, uint32_t neuronIndex, float value);

private:
	// componentwise activation function
	float Activate(float weightedInput) const;

	//contains all weights and biases.
	//weights (connecting a neuron in the previous layer and a neuron in the current layer)
	// indexed by [layerGeneIndex + outputCount + currentNeuron * outputCount + previousNeuron]
	//biases indexed by [layerGeneIndex + biasIndex]
	float* genes;
	// An array used to store activation values. for usage inside of the network
	float* activations;
	// Data about the layers of the network (output neuron count & gene index)
	struct Layer{
		// the index into the gene array
		uint32_t geneIndex;
		// The number of neurons in the layer
		uint32_t outputCount;
	} *layers;
	//Number of layers (not including input layer)
	uint32_t layerCount;
	//Total number of genes
	uint32_t geneCount;
	//Number of input neurons into the first layer
	uint32_t inputCount;
	//The amount the activation is translated in the final activation array
	int activationsTranslation;
	//if the network is initialized
	bool initialized;
};

