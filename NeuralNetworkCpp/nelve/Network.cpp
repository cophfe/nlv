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
	//this will be used internally, so that I don't have to allocate an array for every layer unnecessarily
	//this is terrible code
	activations = new float[maxNeurons * 2];
	activationsTranslation = maxNeurons;
	genes = new float[geneCount];

	initialized = true;
}

Network::~Network()
{
	Uninitialize();
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
	if (initialized)
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
	if (initialized)
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

std::string Network::SaveToString() const
{
	std::ostringstream ss;

	Save(ss);

	// if failed to load string will be empty
	return ss.str();
}

bool Network::SaveToFile(std::string filename) const
{
	std::ofstream file(filename);
	if (!file.is_open())
		return false;

	bool success = Save(file);
	
	file.close();
	return success;
}

bool Network::LoadFromFile(std::string filename)
{
	//open file
	std::ifstream file(filename);
	if (!file.is_open())
		return false;

	bool success = Load(file);
	
	file.close();
	return success;
}

bool Network::LoadFromString(const std::string& string)
{
	std::istringstream ss (string);
	if (ss.fail())
		return false;

	bool success = Load(ss);
	return success;
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
	//sigmoid function
	//this should probably be customizable but eh
	return 1.0f / (1 + exp(weightedInput));
}

bool Network::Save(std::ostream& stream) const
{
	if (!initialized)
		return false;

	//save order:
	// file signiture
	// input count
	// layer count
	// layer data
	// gene count
	// genes

	//NOTE: the last activation values are NOT saved, they need to be recreated by calling Evaluate()
	//it is unnecessary to save the activation values for intended uses of saving and loading
	
	// 8 byte signiture
	// \211 is for the same reason as png 
	// nlvn is for nelve network
	// 000 is for version 000 (yeah I am probably never going to change this)
	stream << "\211NLVN000";
	stream << inputCount << layerCount;
	for (size_t i = 0; i < layerCount; i++)
		stream << layers[i].geneIndex << layers[i].outputCount;
	stream << geneCount;
	for (size_t i = 0; i < geneCount; i++)
		stream << genes[i];

	return true;
}

bool Network::Load(std::istream& stream)
{
	//check header is correct
	std::string header(8, ' ');
	std::copy_n(std::istreambuf_iterator<char>(stream.rdbuf()),
		8, std::back_inserter(header));
	if (header != "\211NLVN000")
		return false;

	//delete contents first if already initialized
	Uninitialize();

	//create network from data
	stream >> inputCount;
	stream >> layerCount;
	layers = (Network::Layer*)malloc(sizeof(Network::Layer) * layerCount);
	uint32_t maxNeurons = 0;
	for (size_t i = 0; i < layerCount; i++)
	{
		stream >> layers[i].geneIndex;
		stream >> layers[i].outputCount;

		maxNeurons = std::max(maxNeurons, layers[i].outputCount);
	}
	stream >> geneCount;
	genes = new float[geneCount];
	for (size_t i = 0; i < geneCount; i++)
		stream >> genes[i];

	//set values
	activations = new float[maxNeurons * 2];
	activationsTranslation = maxNeurons;
	initialized = true;

	return true;
}

void Network::Uninitialize()
{
	if (initialized)
	{
		free(layers);
		delete[] activations;
		delete[] genes;
		layers = nullptr;
		activations = nullptr;
		genes = nullptr;
		layerCount = 0;

		initialized = false;
	}
}
