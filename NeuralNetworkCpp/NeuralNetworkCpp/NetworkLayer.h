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
	/// <summary>
	/// holds the weights connecting a neuron in the previous layer and a neuron in the current layer. indexed by [currentNeuron, previousNeuron]
	/// </summary>
	float* weights;
	/// <summary>
	/// holds neuron bias values
	/// </summary>
	float* biases;

	/// <summary>
	/// The number of neurons in the previous layer
	/// </summary>
	unsigned int inputCount;
	/// <summary>
	/// The number of neurons in the currentLayer
	/// </summary>
	unsigned int outputCount;

	/// <summary>
	/// The last activation values. stored for usage inside of the network
	/// </summary>
	float* activations = nullptr;

	float GetWeight(unsigned int currentNeuronIndex, unsigned int lastNeuronIndex) const;
	void SetWeight(unsigned int currentNeuronIndex, unsigned int lastNeuronIndex, float value);
};
