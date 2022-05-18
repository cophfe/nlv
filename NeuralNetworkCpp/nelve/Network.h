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

	/// <returns>The number of input neurons into the network</returns>
	inline unsigned int GetInputCount() const { return inputCount; }
	/// <returns>The number of output neurons from the network</returns>
	inline unsigned int GetOutputCount() const { return layers[layerCount - 1].outputCount; }

	/// <summary>Randomize the network's values</summary>
	void RandomizeValues();
	/// <summary>Randomize the network's values</summary>
	void RandomizeValues(unsigned int seed);
	/// <summary>Randomize the network's values</summary>
	void RandomizeValues(std::default_random_engine& randEngine);

	float GetWeight(unsigned int layer, unsigned int currentNeuronIndex, unsigned int lastNeuronIndex) const;
	void SetWeight(unsigned int layer, unsigned int currentNeuronIndex, unsigned int lastNeuronIndex, float value);
	float GetBias(unsigned int layer, unsigned int neuronIndex) const;
	void SetBias(unsigned int layer, unsigned int neuronIndex, float value);

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
		unsigned int geneIndex;
		// The number of neurons in the layer
		unsigned int outputCount;
	} *layers;
	//Number of layers (not including input layer)
	unsigned int layerCount;
	//Total number of genes
	unsigned int geneCount;
	//Number of input neurons into the first layer
	unsigned int inputCount;
	//The amount the activation is translated in the final activation array
	int activationsTranslation;
	//if the network is initialized
	bool initialized;
};

