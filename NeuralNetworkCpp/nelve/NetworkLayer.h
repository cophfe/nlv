#pragma once
#include <cstring>
#include <cstdint>
class Network;
class NetworkEvolver;

struct NetworkLayer
{
	friend Network;
	friend NetworkEvolver;

private:
	NetworkLayer(uint32_t inputCount, uint32_t outputCount);
	~NetworkLayer();
	NetworkLayer(const NetworkLayer& other);
	NetworkLayer& operator=(const NetworkLayer& other);
	NetworkLayer(NetworkLayer&& other);
	NetworkLayer& operator=(NetworkLayer&& other);

	// holds the weights connecting a neuron in the previous layer and a neuron in the current layer. 
	// indexed by [currentNeuron, previousNeuron]
	float* weights;
	// holds neuron bias values
	float* biases;
	// The number of neurons in the previous layer
	uint32_t inputCount;
	// The number of neurons in the currentLayer
	uint32_t outputCount;
	// The last activation values. stored for usage inside of the network
	float* activations = nullptr;

	float GetWeight(uint32_t currentNeuronIndex, uint32_t lastNeuronIndex) const;
	void SetWeight(uint32_t currentNeuronIndex, uint32_t lastNeuronIndex, float value);
};
