#include "Network.h"

Network::Network(unsigned int inputNeurons, std::initializer_list<unsigned int> hiddenLayerNeurons, unsigned int outputNeurons)
{
	if (inputNeurons == 0 || outputNeurons == 0)
		throw std::runtime_error("Neuron count cannot be less than or equal to 0");

	layerCount = hiddenLayerNeurons.size() + 1;
	layers = new NetworkLayer[layerCount];
	unsigned int inputCount = inputNeurons;

	size_t i = 0;
	for (auto&& layerCount : hiddenLayerNeurons)
	{
		if (layerCount == 0)
			throw std::runtime_error("Neuron count cannot be less than or equal to 0");
		
		layers[i].Init(inputCount, layerCount);
		i++;
	}
	layers[layerCount - 1].Init(inputCount, outputNeurons);
}

Network::~Network()
{
	if (layers != nullptr)
	{
		delete[] layers;
		layers = nullptr;
	}
	layerCount = 0;
}

Network::Network(const Network& other)
{
	layerCount = other.layerCount;
	layers = new NetworkLayer[layerCount];
	for (size_t i = 0; i < layerCount; i++)
	{
		layers[i] = other.layers[i];
	}
}

Network::Network(Network&& other)
{
	layers = other.layers;
	layerCount = other.layerCount;

	other.layerCount = 0;
	other.layers = nullptr;
}

Network& Network::operator=(const Network& other)
{
	if (layers != nullptr)
		delete[] layers;

	layerCount = other.layerCount;
	layers = new NetworkLayer[layerCount];
	for (size_t i = 0; i < layerCount; i++)
	{
		layers[i] = other.layers[i];
	}
	return *this;
}

Network& Network::operator=(Network&& other)
{
	if (layers != nullptr)
		delete[] layers;

	layers = other.layers;
	layerCount = other.layerCount;

	other.layerCount = 0;
	other.layers = nullptr;
	return *this;
}

float const* Network::Evaluate(float* input, unsigned int inputCount)
{
	if (input == nullptr)
		throw std::runtime_error("Cannot pass a null value as input");
	if (inputCount != layers[0].inputCount)
		throw std::runtime_error("Incorrect number of inputs");

	//iterate over the layers, each layer taking the previous layer's output as input
	for (size_t l = 0; l < layerCount; l++)
	{
		//calculate every neuron's activation value
		for (size_t n = 0; n < layers[l].outputCount; n++)
		{
			float weightedInput = layers[l].biases[n];
			for (int w = 0; w < inputCount; w++)
				weightedInput += input[w] * layers[l].GetWeight(n, w);
			layers[l].activations[n] = Activate(weightedInput);
		}

		//set values for the next layer
		input = layers[l].activations;
		inputCount = layers[l].outputCount;
	}

	//return the networks outputs
	return layers[layerCount - 1].activations;
}

float const* Network::GetPreviousActivations() const
{
	return layers[layerCount - 1].activations;
}

void Network::RandomizeValues()
{
	std::random_device rand;
	std::default_random_engine rEngine(rand);
	std::normal_distribution<float> dist(0, 1);

	for (size_t l = 0; l < layerCount; l++)
	{
		for (size_t w = 0; w < layers[l].inputCount * layers[l].outputCount; w++)
		{
			layers[l].weights[w] = dist(rEngine);
		}
		for (size_t b = 0; b < layers[l].outputCount; b++)
		{
			layers[l].biases[b] = dist(rEngine);
		}
	}
}

void Network::RandomizeValues(unsigned int seed)
{
	std::default_random_engine rEngine(seed);
	std::normal_distribution<float> dist(0, 1);

	for (size_t l = 0; l < layerCount; l++)
	{
		for (size_t w = 0; w < layers[l].inputCount * layers[l].outputCount; w++)
		{
			layers[l].weights[w] = dist(rEngine);
		}
		for (size_t b = 0; b < layers[l].outputCount; b++)
		{
			layers[l].biases[b] = dist(rEngine);
		}
	}
}

void Network::RandomizeValues(const std::default_random_engine& randEngine)
{
	std::normal_distribution<float> dist(0, 1);

	for (size_t l = 0; l < layerCount; l++)
	{
		for (size_t w = 0; w < layers[l].inputCount * layers[l].outputCount; w++)
		{
			layers[l].weights[w] = dist(randEngine);
		}
		for (size_t b = 0; b < layers[l].outputCount; b++)
		{
			layers[l].biases[b] = dist(randEngine);
		}
	}
}

float Network::Activate(float weightedInput) const
{
	//sigmoid
	return 1.0f / (1 + exp(weightedInput));
}
