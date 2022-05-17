#include "Network.h"

Network::Network(unsigned int inputNeurons, std::initializer_list<unsigned int> hiddenLayerNeurons, unsigned int outputNeurons)
	: inputCount(inputNeurons)
{
	if (inputNeurons == 0 || outputNeurons == 0)
		throw std::runtime_error("Neuron count cannot be less than or equal to 0");

	layerCount = hiddenLayerNeurons.size() + 1;

	//create an uninitialized array of layers
	layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);

	//get the input count for the first layer
	unsigned int inputCount = inputNeurons;
	//find the number of genes in the network
	unsigned int geneCount = 0;

	unsigned int maxNeurons = 0;
	size_t i = 0;
	for (auto&& layerOutputs : hiddenLayerNeurons)
	{
		if (layerOutputs == 0)
			throw std::runtime_error("Neuron count cannot be less than or equal to 0");
		
		//setup layer values
		layers[i].geneIndex = geneCount;
		layers[i].outputCount = layerOutputs;
		geneCount += (inputCount + 1) * layerOutputs;
		
		//find max neurons
		maxNeurons = std::max(maxNeurons, layerOutputs);
		inputCount = layerOutputs;
		i++;
	}
	//setup output layer values
	layers[i].geneIndex = geneCount;
	layers[i].outputCount = outputNeurons;
	geneCount += (inputCount + 1) * outputNeurons;
	maxNeurons = std::max(maxNeurons, outputNeurons);

	//with the max amount of neurons, create an activations array that can hold any layer's activations
	//this will be used internally, so that I don't have to allocate an array for every layer
	//this is probably considered to be terrible code
	activations = new float[maxNeurons * 2];
	activationsTranslation = maxNeurons;
	genes = new float[geneCount];
}

Network::~Network()
{
	if (layers)
	{
		free(layers);
		delete[] activations;
		delete[] genes;
		layers = nullptr;
		activations = nullptr;
		genes = nullptr;
	}
	layerCount = 0;
}

Network::Network(const Network& other)
	: layerCount(other.layerCount), inputCount(other.inputCount), geneCount(other.geneCount), activationsTranslation(other.activationsTranslation)
{
	layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);
	if (!layers)
		throw std::runtime_error("Failed to allocate data for layer");
	memcpy(layers, other.layers, sizeof(Network::Layer) * layerCount);

	activations = new float[activationsTranslation * 2];
	memcpy(activations, other.activations, activationsTranslation * 2);
	
	genes = new float[geneCount];
	memcpy(genes, other.genes, geneCount);
}

Network::Network(Network&& other)
	: layerCount(other.layerCount), inputCount(other.inputCount), geneCount(other.geneCount), activationsTranslation(other.activationsTranslation)
{
	layers = other.layers;
	activations = other.activations;
	genes = other.genes;

	other.layerCount = 0;
	other.layers = nullptr;
	other.genes = nullptr;
	other.activations = nullptr;
}

Network& Network::operator=(const Network& other)
{
	if (layers)
	{
		free(layers);
		delete[] activations;
		delete[] genes;
	}

	layerCount = other.layerCount;
	activationsTranslation = other.activationsTranslation;
	inputCount = other.inputCount;
	geneCount = other.geneCount;
	layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);
	if (!layers)
		throw std::runtime_error("Failed to allocate data for layer");
	memcpy(layers, other.layers, sizeof(Network::Layer) * layerCount);
	activations = new float[activationsTranslation * 2];
	memcpy(activations, other.activations, activationsTranslation * 2);
	genes = new float[geneCount];
	memcpy(genes, other.genes, geneCount);
	return *this;
}

Network& Network::operator=(Network&& other)
{
	if (layers)
	{
		free(layers);
		delete[] activations;
		delete[] genes;
	}

	layerCount = other.layerCount;
	activationsTranslation = other.activationsTranslation;
	inputCount = other.inputCount;
	geneCount = other.geneCount;
	layers = other.layers;
	activations = other.activations;
	genes = other.genes;

	other.layerCount = 0;
	other.layers = nullptr;
	other.genes = nullptr;
	other.activations = nullptr;
	return *this;
}

float const* Network::Evaluate(float* input, unsigned int inputCount)
{
#ifdef _DEBUG
	if (input == nullptr)
		throw std::runtime_error("Cannot pass a null value as input");
	if (inputCount != inputCount)
		throw std::runtime_error("Incorrect number of inputs");
#endif
	//make sure the final layer output isn't offset from the activations array
	unsigned int t = layerCount % 2 == 0 ? 0 : activationsTranslation;

	//iterate over the layers, each layer taking the previous layer's output as input
	//both input and output are stored in the activations array
	float* output = activations + t;
	for (size_t l = 0; l < layerCount; l++)
	{
		//calculate every neuron's activation value
		for (size_t n = 0; n < layers[l].outputCount; n++)
		{
			float weightedInput = GetBias(l, n);
			for (int w = 0; w < inputCount; w++)
				weightedInput += input[w] * GetWeight(l, n, w);
			output[n] = Activate(weightedInput);
		}

		inputCount = layers[l].outputCount;
		//swap input and output arrays
		input = output;
		output = activations + (activationsTranslation - t);
		//if t == 0, set t to activationsTranslation. else set to 0 
		t = t == 0 ? activationsTranslation : 0;
	}

	//return the networks outputs
	return activations;
}

float const* Network::GetPreviousActivations() const
{
	return activations;
}

void Network::RandomizeValues()
{
	std::random_device rand;
	std::default_random_engine rEngine(rand());
	std::normal_distribution<float> dist(0, 1);
	for (size_t i = 0; i < geneCount; i++)
		genes[i] = dist(rEngine);
}

void Network::RandomizeValues(unsigned int seed)
{
	std::default_random_engine rEngine(seed);
	std::normal_distribution<float> dist(0, 1);
	for (size_t i = 0; i < geneCount; i++)
		genes[i] = dist(rEngine);
}

void Network::RandomizeValues(std::default_random_engine& randEngine)
{
	std::normal_distribution<float> dist(0, 1);
	for (size_t i = 0; i < geneCount; i++)
		genes[i] = dist(randEngine);
}

float Network::GetWeight(unsigned int layer, unsigned int currentNeuron, unsigned int previousNeuron) const
{
#ifdef _DEBUG
	if (layer >= layerCount)
		throw std::runtime_error("Layer index exceeds the layer count");
	if (currentNeuron >= layers[layer].outputCount)
		throw std::runtime_error("Neuron index exceeds the neuron count");
	if (previousNeuron >= (layer == 0 ? inputCount : layers[layer - 1].outputCount))
		throw std::runtime_error("Neuron index exceeds the neuron count");
#endif

	Network::Layer& l = layers[layer];
	//return weight at index [currentNeuronIndex, lastNeuronIndex] at specified layer
	return genes[l.geneIndex + l.outputCount + previousNeuron * l.outputCount + currentNeuron];
}

void Network::SetWeight(unsigned int layer, unsigned int currentNeuron, unsigned int previousNeuron, float value)
{
	Network::Layer& l = layers[layer];
	//set weight at index [currentNeuronIndex, lastNeuronIndex] at specified layer
	genes[l.geneIndex + l.outputCount + previousNeuron * l.outputCount + currentNeuron] = value;
}

float Network::GetBias(unsigned int layer, unsigned int neuronIndex) const
{
#ifdef _DEBUG
	if (layer >= layerCount)
		throw std::runtime_error("Layer index exceeds the layer count");
	if (neuronIndex >= layers[layer].outputCount)
		throw std::runtime_error("Neuron index exceeds the neuron count");
#endif

	return genes[layers[layer].geneIndex + neuronIndex];
}

void Network::SetBias(unsigned int layer, unsigned int neuronIndex, float value)
{
#ifdef _DEBUG
	if (layer > layerCount)
		throw std::runtime_error("Layer index exceeds the layer count");
	if (neuronIndex > layers[layer].outputCount)
		throw std::runtime_error("Neuron index exceeds the neuron count");
#endif

	genes[layers[layer].geneIndex + neuronIndex] = value;
}

float Network::Activate(float weightedInput) const
{
	//sigmoid
	return 1.0f / (1 + exp(weightedInput));
}
