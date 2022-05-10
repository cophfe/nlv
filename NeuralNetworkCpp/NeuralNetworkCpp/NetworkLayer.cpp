#include "NetworkLayer.h"

NetworkLayer::~NetworkLayer()
{
	if (activations != nullptr)
	{
		delete[] weights;
		delete[] biases;
		delete[] activations;

		weights = nullptr;
		biases = nullptr;
		activations = nullptr;
	}
}
NetworkLayer::NetworkLayer(const NetworkLayer& other)
{
	outputCount = other.outputCount;
	inputCount = other.inputCount;
	
	if (other.activations != nullptr)
	{
		weights = new float[inputCount * outputCount];
		biases = new float[outputCount];
		activations = new float[outputCount];

		memcpy(weights, other.weights, inputCount * outputCount);
		memcpy(biases, other.biases, outputCount);
		memcpy(activations, other.activations, outputCount);
	}
}
NetworkLayer& NetworkLayer::operator=(const NetworkLayer& other)
{
	if (activations != nullptr)
	{
		delete[] weights;
		delete[] biases;
		delete[] activations;

		weights = nullptr;
		biases = nullptr;
		activations = nullptr;
	}

	outputCount = other.outputCount;
	inputCount = other.inputCount;

	if (other.activations != nullptr)
	{
		weights = new float[inputCount * outputCount];
		biases = new float[outputCount];
		activations = new float[outputCount];

		memcpy(weights, other.weights, inputCount * outputCount);
		memcpy(biases, other.biases, outputCount);
		memcpy(activations, other.activations, outputCount);
	}
	return *this;
}

void NetworkLayer::Init(unsigned int inputCount, unsigned int outputCount)
{
	//create arrays
	weights = new float[inputCount * outputCount];
	biases = new float[outputCount];
	activations = new float[outputCount];
}

float NetworkLayer::GetWeight(unsigned int currentNeuronIndex, unsigned int lastNeuronIndex) const
{
	//return weight at index [currentNeuronIndex, lastNeuronIndex]
	return weights[lastNeuronIndex * outputCount + currentNeuronIndex];
}

void NetworkLayer::SetWeight(unsigned int currentNeuronIndex, unsigned int lastNeuronIndex, float value)
{
	//set weight at index [currentNeuronIndex, lastNeuronIndex] to value
	weights[lastNeuronIndex * outputCount + currentNeuronIndex] = value;
}
