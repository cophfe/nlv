#pragma once
#include <cstring>
class Network;

struct NetworkLayer
{
private:
	friend Network;

	NetworkLayer() = default;
	~NetworkLayer();
	NetworkLayer(const NetworkLayer& other);
	NetworkLayer& operator=(const NetworkLayer& other);

	void Init(unsigned int inputCount, unsigned int outputCount);
	
	// holds the weights connecting a neuron in the previous layer and a neuron in the current layer. 
	// indexed by [currentNeuron, previousNeuron]
	float* weights;
	// holds neuron bias values
	float* biases;
	// The number of neurons in the previous layer
	unsigned int inputCount;
	// The number of neurons in the currentLayer
	unsigned int outputCount;
	// The last activation values. stored for usage inside of the network
	float* activations = nullptr;

	float GetWeight(unsigned int currentNeuronIndex, unsigned int lastNeuronIndex) const;
	void SetWeight(unsigned int currentNeuronIndex, unsigned int lastNeuronIndex, float value);
};
