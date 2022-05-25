#include "Network.h"

Network::Network()
{
	inputCount = 0;
	layerCount = 0;
	geneCount = 0;
	layers = nullptr;
	genes = nullptr;
	activations = nullptr;
	activationsTranslation = 0;
	initialized = false;
}

Network::Network(int inputNeurons, std::initializer_list<int> hiddenLayerNeurons, int outputNeurons)
	: inputCount(inputNeurons)
{
	if (inputNeurons <= 0 || outputNeurons <= 0)
		throw std::runtime_error("Neuron count cannot be less than or equal to 0");

	layerCount = hiddenLayerNeurons.size() + 1;

	//create an uninitialized array of layers
	layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);

	//get the input count for the first layer
	uint32_t inputCount = inputNeurons;
	//find the number of genes in the network
	geneCount = 0;

	int maxNeurons = 0;
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

	initialized = true;
}

Network::Network(int inputNeurons, std::vector<int> hiddenLayerNeurons, int outputNeurons)
	: inputCount(inputNeurons)
{
	//literal copy paste but with vector instead of initializer list

	if (inputNeurons <= 0 || outputNeurons <= 0)
		throw std::runtime_error("Neuron count cannot be less than or equal to 0");

	layerCount = hiddenLayerNeurons.size() + 1;

	//create an uninitialized array of layers
	layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);

	//get the input count for the first layer
	uint32_t inputCount = inputNeurons;
	//find the number of genes in the network
	geneCount = 0;

	int maxNeurons = 0;
	size_t i = 0;
	for (auto&& layerOutputs : hiddenLayerNeurons)
	{
		if (layerOutputs <= 0)
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

	initialized = true;
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
	: layerCount(other.layerCount), inputCount(other.inputCount), geneCount(other.geneCount), activationsTranslation(other.activationsTranslation),
	initialized(other.initialized)
{
	layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);
	if (!layers)
		throw std::runtime_error("Failed to allocate data for layer");
	memcpy(layers, other.layers, sizeof(Network::Layer) * layerCount);

	activations = new float[activationsTranslation * 2];
	memcpy(activations, other.activations, sizeof(float) * activationsTranslation * 2);
	
	genes = new float[geneCount];
	memcpy(genes, other.genes, sizeof(float) * geneCount);
}

Network::Network(Network&& other)
	: layerCount(other.layerCount), inputCount(other.inputCount), geneCount(other.geneCount), activationsTranslation(other.activationsTranslation), 
	initialized(other.initialized), layers(other.layers), activations(other.activations), genes(other.genes)
{
	other.layerCount = 0;
	other.layers = nullptr;
	other.genes = nullptr;
	other.activations = nullptr;
	other.initialized = false;
}

Network& Network::operator=(const Network& other)
{
	if (layers)
	{
		free(layers);
		delete[] activations;
		delete[] genes;
	}

	initialized = other.initialized;
	layerCount = other.layerCount;
	activationsTranslation = other.activationsTranslation;
	inputCount = other.inputCount;
	geneCount = other.geneCount;
	layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);
	if (!layers)
		throw std::runtime_error("Failed to allocate data for layer");
	memcpy(layers, other.layers, sizeof(Network::Layer) * layerCount);
	activations = new float[activationsTranslation * 2];
	memcpy(activations, other.activations, sizeof(float) * activationsTranslation * 2);
	genes = new float[geneCount];
	memcpy(genes, other.genes, sizeof(float) * geneCount);
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
	initialized = other.initialized;

	other.layerCount = 0;
	other.layers = nullptr;
	other.genes = nullptr;
	other.activations = nullptr;
	other.initialized = false;
	return *this;
}

Network Network::CloneNetworkLayout()
{
	Network network;
	network.layerCount = layerCount;
	network.activationsTranslation = activationsTranslation;
	network.inputCount = inputCount;
	network.geneCount = geneCount;
	network.initialized = initialized;

	network.layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);
	if (!network.layers)
		throw std::runtime_error("Failed to allocate data for layer");
	memcpy(network.layers, layers, sizeof(Network::Layer) * layerCount);
	network.activations = new float[activationsTranslation * 2];
	network.genes = new float[geneCount];
	return network;
}

float const* Network::Evaluate(float* input, uint32_t inputCount)
{
#ifdef _DEBUG
	if (input == nullptr)
		throw std::runtime_error("Cannot pass a null value as input");
	if (inputCount != inputCount)
		throw std::runtime_error("Incorrect number of inputs");
	if (!initialized)
		throw std::runtime_error("Can not evaluate an uninitialized network");
#endif
	//make sure the final layer output isn't offset from the activations array
	uint32_t t = layerCount % 2 == 0 ? 0 : activationsTranslation;

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
#ifdef _DEBUG
	if (!initialized)
		throw std::runtime_error("Can not read from an uninitialized network");
#endif

	return activations;
}

void Network::RandomizeValues()
{
#ifdef _DEBUG
	if (!initialized)
		throw std::runtime_error("Can not set values in an uninitialized network");
#endif

	std::random_device rand;
	std::default_random_engine rEngine(rand());
	std::normal_distribution<float> dist(0, 1);
	for (size_t i = 0; i < geneCount; i++)
		genes[i] = dist(rEngine);
}

void Network::RandomizeValues(uint32_t seed)
{
#ifdef _DEBUG
	if (!initialized)
		throw std::runtime_error("Can not set values in an uninitialized network");
#endif

	std::default_random_engine rEngine(seed);
	std::normal_distribution<float> dist(0, 1);
	for (size_t i = 0; i < geneCount; i++)
		genes[i] = dist(rEngine);
}

void Network::RandomizeValues(std::default_random_engine& randEngine)
{
#ifdef _DEBUG
	if (!initialized)
		throw std::runtime_error("Can not set values in an uninitialized network");
#endif

	std::normal_distribution<float> dist(0, 1);
	for (size_t i = 0; i < geneCount; i++)
		genes[i] = dist(randEngine);
}

float Network::GetWeight(uint32_t layer, uint32_t currentNeuron, uint32_t previousNeuron) const
{
#ifdef _DEBUG
	if (layer >= layerCount)
		throw std::runtime_error("Layer index exceeds the layer count");
	if (currentNeuron >= layers[layer].outputCount)
		throw std::runtime_error("Neuron index exceeds the neuron count");
	if (previousNeuron >= (layer == 0 ? inputCount : layers[layer - 1].outputCount))
		throw std::runtime_error("Neuron index exceeds the neuron count");
	if (!initialized)
		throw std::runtime_error("Can not read values from an uninitialized network");
#endif

	Network::Layer& l = layers[layer];
	//return weight at index [currentNeuronIndex, lastNeuronIndex] at specified layer
	return genes[l.geneIndex + l.outputCount + previousNeuron * l.outputCount + currentNeuron];
}

void Network::SetWeight(uint32_t layer, uint32_t currentNeuron, uint32_t previousNeuron, float value)
{
#ifdef _DEBUG
	if (layer >= layerCount)
		throw std::runtime_error("Layer index exceeds the layer count");
	if (currentNeuron >= layers[layer].outputCount)
		throw std::runtime_error("Neuron index exceeds the neuron count");
	if (previousNeuron >= (layer == 0 ? inputCount : layers[layer - 1].outputCount))
		throw std::runtime_error("Neuron index exceeds the neuron count");
	if (!initialized)
		throw std::runtime_error("Can not read values from an uninitialized network");
#endif

	Network::Layer& l = layers[layer];
	//set weight at index [currentNeuronIndex, lastNeuronIndex] at specified layer
	genes[l.geneIndex + l.outputCount + previousNeuron * l.outputCount + currentNeuron] = value;
}

float Network::GetBias(uint32_t layer, uint32_t neuronIndex) const
{
#ifdef _DEBUG
	if (layer >= layerCount)
		throw std::runtime_error("Layer index exceeds the layer count");
	if (neuronIndex >= layers[layer].outputCount)
		throw std::runtime_error("Neuron index exceeds the neuron count");
	if (!initialized)
		throw std::runtime_error("Can not read values from an uninitialized network");
#endif

	return genes[layers[layer].geneIndex + neuronIndex];
}

void Network::SetBias(uint32_t layer, uint32_t neuronIndex, float value)
{
#ifdef _DEBUG
	if (layer > layerCount)
		throw std::runtime_error("Layer index exceeds the layer count");
	if (neuronIndex > layers[layer].outputCount)
		throw std::runtime_error("Neuron index exceeds the neuron count");
	if (!initialized)
		throw std::runtime_error("Can not read values from an uninitialized network");
#endif

	genes[layers[layer].geneIndex + neuronIndex] = value;
}

float Network::Activate(float weightedInput) const
{
	//sigmoid
	return 1.0f / (1 + exp(weightedInput));
}
