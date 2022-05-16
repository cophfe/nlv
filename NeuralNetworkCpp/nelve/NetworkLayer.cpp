#include "NetworkLayer.h"

NetworkLayer::NetworkLayer(unsigned int inputCount, unsigned int outputCount)
{
	this->inputCount = inputCount;
	this->outputCount = outputCount;

	weights = new float[inputCount * outputCount];
	biases = new float[outputCount];
	activations = new float[outputCount];
}

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

NetworkLayer::NetworkLayer(NetworkLayer&& other)
{
	outputCount = other.outputCount;
	inputCount = other.inputCount;
	
	weights = other.weights;
	biases = other.biases;
	activations = other.activations;

	other.activations = nullptr;
	other.biases = nullptr;
	other.weights = nullptr;
}

NetworkLayer& NetworkLayer::operator=(NetworkLayer&& other)
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

	weights = other.weights;
	biases = other.biases;
	activations = other.activations;

	other.activations = nullptr;
	other.biases = nullptr;
	other.weights = nullptr;
	return *this;
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
