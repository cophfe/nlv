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

	weights = new float[inputCount * outputCount];
	biases = new float[outputCount];
	activations = new float[outputCount];

	memcpy(weights, other.weights, inputCount * outputCount);
	memcpy(biases, other.weights, outputCount);
	memcpy(activations, other.weights, outputCount);
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

	weights = new float[inputCount * outputCount];
	biases = new float[outputCount];
	activations = new float[outputCount];

	memcpy(weights, other.weights, inputCount * outputCount);
	memcpy(biases, other.weights, outputCount);
	memcpy(activations, other.weights, outputCount);
}

void NetworkLayer::Init(unsigned int inputCount, unsigned int outputCount)
{
	weights = new float[inputCount * outputCount];
	biases = new float[outputCount];
	activations = new float[outputCount];
}

float NetworkLayer::GetWeight(unsigned int currentNeuronIndex, unsigned int lastNeuronIndex) const
{
	return weights[lastNeuronIndex * outputCount + currentNeuronIndex];
}

void NetworkLayer::SetWeight(unsigned int currentNeuronIndex, unsigned int lastNeuronIndex, float value)
{
	weights[lastNeuronIndex * outputCount + currentNeuronIndex] = value;
}
