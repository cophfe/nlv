#include "Network.h"

Network::Network(unsigned int inputNeurons, std::initializer_list<unsigned int> hiddenLayerNeurons, unsigned int outputNeurons)
{
	if (inputNeurons == 0 || outputNeurons == 0)
		throw std::runtime_error("Neuron count cannot be less than or equal to 0");

	layerCount = hiddenLayerNeurons.size() + 1;

	//uninitialized array
	layers = (NetworkLayer*)malloc(sizeof(NetworkLayer) * layerCount);
	//layers = (NetworkLayer*)(::operator new(sizeof(NetworkLayer) * layerCount));

	unsigned int inputCount = inputNeurons;

	size_t i = 0;
	for (auto&& layerCount : hiddenLayerNeurons)
	{
		if (layerCount == 0)
			throw std::runtime_error("Neuron count cannot be less than or equal to 0");
		
		//construct without copying (copy has issues on uninitialized data)
		new (layers + i) NetworkLayer(inputCount, layerCount);

		inputCount = layerCount;
		i++;
	}
	new (layers + (layerCount - 1)) NetworkLayer(inputCount, outputNeurons);

	//also consider this-> https://stackoverflow.com/questions/16316308/c-uninitialized-array-of-class-instances
}

Network::~Network()
{
	if (layers != nullptr)
	{
		for (size_t i = 0; i < layerCount; i++)
			(*(layers + i)).~NetworkLayer();
		free(layers);
	}
	layerCount = 0;
}

Network::Network(const Network& other)
{
	layerCount = other.layerCount;
	//make the layers array with all bytes initialized to zero
	//needs to be set to zero for the copy constructor not to throw an error
	layers = (NetworkLayer*)(new char[sizeof(NetworkLayer) * layerCount]());
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
	{
		for (size_t i = 0; i < layerCount; i++)
			(*(layers + i)).~NetworkLayer();
		free(layers);
	}

	layerCount = other.layerCount;
	layers = (NetworkLayer*)(new char[sizeof(NetworkLayer) * layerCount]());
	for (size_t i = 0; i < layerCount; i++)
	{
		layers[i] = other.layers[i];
	}
	return *this;
}

Network& Network::operator=(Network&& other)
{
	if (layers != nullptr)
	{
		for (size_t i = 0; i < layerCount; i++)
			(*(layers + i)).~NetworkLayer();
		free(layers);

		layers = nullptr;
	}

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
	std::default_random_engine rEngine(rand());

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

void Network::RandomizeValues(std::default_random_engine& randEngine)
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
