//#pragma once

//class Layer
//{
//	public:
//	Layer(const Layer& previousLayer, unsigned int nodeCount)
//		: inputArray(previousLayer.GetOutputArray()), inputCount(previousLayer.GetNodeCount()), nodeCount(nodeCount)
//	{
//		biasArray = new float[nodeCount];
//		outputArray = new float[nodeCount];
//		weightsArray = new float[nodeCount * inputCount];
//	}

//	Layer(float* const& inputArray, unsigned int inputCount, unsigned int nodeCount)
//		: inputArray(inputArray), inputCount(inputCount), nodeCount(nodeCount)
//	{
//		biasArray = new float[nodeCount];
//		outputArray = new float[nodeCount];
//		weightsArray = new float[nodeCount * inputCount];
//	}

//	inline float* const& GetOutputArray() const { return outputArray; }
//inline unsigned int GetNodeCount() const { return nodeCount; }
//private:
//	float* const& inputArray;
//unsigned int inputCount;

//float* outputArray;
//float* biasArray;
//float* weightsArray;
//unsigned int nodeCount;
//};

//class Network
//{

//	private:
//	Layer layer;
//};

