#pragma once
#include <random>
#include <fstream>

namespace nlv
{
	class NetworkEvolver;

	//a feed forward neural network
	class Network
	{
	public:
		Network(); // <--this initializes an empty network, which will not be able to do anything
		Network(int inputs, std::vector<int> hiddenLayerNeurons, int outputs);
		Network(const Network& other);
		Network(Network&& other);
		Network& operator=(const Network& other);
		Network& operator=(Network&& other);
		~Network();

		// Saves the network data to a string
		// returns a string with all the required information about the network
		std::string SaveToString() const;
		
		// Saves the network data to a file
		// filename: The filename of which to save the network to
		// Returns whether the save succeeded or failed
		bool SaveToFile(std::string filename) const;

		// Loads a network from a file
		// filename: The filename of which to load the network from
		// Returns whether the load succeeded or failed
		bool LoadFromFile(std::string filename);
		
		// Loads a network from a string
		// string: The string of which to load the network from
		// Returns whether the load succeeded or failed
		bool LoadFromString(const std::string& string);

		// input: The activations of the input layer
		// inputCount: The number of neurons in the input layer
		// returns the output activations of the neural network (always values between 0 and 1)
		float const* Evaluate(float* input, uint32_t inputCount);

		// Returns the last output activations calculated by the evaluate function (always values between 0 and 1)
		float const* GetPreviousActivations() const;

		// Returns the number of input neurons into the network
		inline uint32_t GetInputCount() const { return inputCount; }
		
		// Returns the number of output neurons from the network
		inline uint32_t GetOutputCount() const { return layers[layerCount - 1].outputCount; }

		// Randomizes the network's values
		void RandomizeValues();
		// seed: used to seed the random engine
		void RandomizeValues(uint32_t seed);
		// randEngine: the random engine used to randomize the network
		void RandomizeValues(std::default_random_engine& randEngine);

		// layer: the layer the current neuron is on
		// currentNeuronIndex: the index of the neuron in the specified layer
		// lastNeuronIndex: the index of the neuron in the layer before the specified one
		// Returns the weight value between the last and current neuron
		float GetWeight(uint32_t layer, uint32_t currentNeuronIndex, uint32_t lastNeuronIndex) const;
		
		// Sets the weight value between the last and current neuron
		void SetWeight(uint32_t layer, uint32_t currentNeuronIndex, uint32_t lastNeuronIndex, float value);

		// layer: the layer the neuron is on
		// neuronIndex: the index of the neuron
		// Returns the bias value of a neuron
		float GetBias(uint32_t layer, uint32_t neuronIndex) const;
		
		// Sets the bias value of a neuron
		void SetBias(uint32_t layer, uint32_t neuronIndex, float value);

	private:
		//network evolver is allowed to modify the genes directly
		friend NetworkEvolver;

		// Componentwise activation function (specifically a sigmoid function)
		float Activate(float weightedInput) const;

		//save to stream
		bool Save(std::ostream& stream) const;
		//load from stream
		bool Load(std::istream& stream);

		//delete the network (called by load functions, copy and move assigners and destructor)
		void Uninitialize();

		//contains all weights and biases.
		//weights (connecting a neuron in the previous layer and a neuron in the current layer)
		// indexed by [layerGeneIndex + outputCount + currentNeuron * outputCount + previousNeuron]
		//biases indexed by [layerGeneIndex + biasIndex]
		float* genes;
		// An array used to store the last activation output values
		float* activations;
		// Data about the layers of the network (output neuron count & gene index)
		struct Layer {
			// the index into the gene array
			uint32_t geneIndex;
			// The number of neurons in the layer
			uint32_t outputCount;
		} *layers;
		//Number of layers (not including input layer)
		uint32_t layerCount;
		//Total number of genes
		uint32_t geneCount;
		//Number of neurons into the input layer
		uint32_t inputCount;
		//The amount the activation is translated in the final activation array (used internally)
		int activationsTranslation;
		//if the network is initialized 
		bool initialized;
	};
}