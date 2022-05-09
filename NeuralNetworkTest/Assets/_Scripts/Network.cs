using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class Network
{
	Layer[] layers;
	Layer LastLayer => layers[layers.Length - 1];

	public Network(int initialInputCount, params int[] neuronsInLayer)
	{
		int inputCount = initialInputCount;
		layers = new Layer[neuronsInLayer.Length];
		for (int i = 0; i < neuronsInLayer.Length; i++)
		{
			layers[i] = new Layer(neuronsInLayer[i], inputCount);
			inputCount = neuronsInLayer[i];
		}
	}

	public float[] Evaluate(float[] inputs)
	{
		if (inputs.Length != layers[0].InputCount)
		{
			Debug.LogError("Incorrect number of inputs into neural network.");
			return null;
		}

		foreach (var layer in layers)
		{
			layer.Evaluate(inputs);
			inputs = layer.Activations;
		}

		return LastLayer.Activations;
	}

	//takes correct input-output pairs and uses them to modify all weights and biases using gradient descent + backpropagation
	//note: this technique performs horribly when their are a lot of layers
	public void Train(int batchSize, CorrectInputOutputPair[] trainingData, float learningRate)
	{
		//for thread safety since this runs on a seperate thread
		System.Random rand = new System.Random();
		CorrectInputOutputPair[] randomized = trainingData.OrderBy(x => rand.NextDouble()).ToArray();
		
		//this shuffle requires too many recurses and causes a stackoverflow lol
		//Shuffle(trainingData);

		//will ignore the last few if batchsize does not divide into training data, but that isn't a big issue
		int batches = trainingData.Length / batchSize;
		for (int i = 0; i < batches; i++)
		{
			//Stochastic gradient descent batch
			Batch(trainingData, i, batchSize, learningRate); //if I do this it should get really good at this one batch example
		}
	}

	void Shuffle(CorrectInputOutputPair[] array)
	{
		//O(n) complexity shuffle: https://stackoverflow.com/questions/108819/best-way-to-randomize-an-array-with-net
		ShuffleRecurse(array.Length);
		
		void ShuffleRecurse(int n)
		{
			if (n > 0)
			{
				ShuffleRecurse(n - 1);
				int randomIndex = UnityEngine.Random.Range(0, n);
				CorrectInputOutputPair value = array[randomIndex];
				array[randomIndex] = array[n - 1];
				array[n - 1] = value;
			}
		}
	}

	//tuples are cool
	//(int, float, Vector2) Cool()
	//{
	//	return (1, 2f, Vector2.zero);
	//}

	void Batch(CorrectInputOutputPair[] randomizedTrainingData, int batchIndex, int batchSize, float learningRate)
	{

		//https://youtu.be/tIeHLnjs5U8
		//weight gradient: the magnitude and direction of change to neuron weights that lead to a higher cost value
		//error: the magnitude and direction of change to bias weights 
		//organised like this instead of a 1D array for ease of access, in theory it is still a n dimensional gradient value
		float[][,] weightGradients = new float[layers.Length][,];
		float[][] errors = new float[layers.Length][]; //errors are the same as the bias' gradients

		//local errors are needed for calculating weight gradients.
		float[][] localErrors = new float[layers.Length][]; 

		for (int i = 0; i < layers.Length; i++)
		{
			//for every neuron there are InputCount weights 
			weightGradients[i] = new float[layers[i].NeuronCount, layers[i].InputCount];
			//also one bias per neuron
			errors[i] = new float[layers[i].NeuronCount];
			localErrors[i] = new float[layers[i].NeuronCount];
		}

		for (int d = 0; d < batchSize; d++)
		{
			CorrectInputOutputPair data = randomizedTrainingData[batchIndex * batchSize + d];

			Evaluate(data.input);

			//use backpropagation to get the cost gradient
			//neuron's bias cost gradient = SigmoidDerivitive(unactivatedOutput[i]) * CostDerivitive(output[i], correctOutput[i]);
			//neuron's weight cost gradient = previousLayer.weights * SigmoidDerivitive(unactivatedOutput[i]) * CostDerivitive(output[i], correctOutput[i]);

			//get the cost of each output
			//int outputCount = LastLayer.NeuronCount;
			//float[] cost = new float[LastLayer.NeuronCount];
			//for (int i = 0; i < outputCount; i++)
			//{
			//	cost[i] = CostFunction(LastLayer.Activations[i], data.activations[i]);
			//}


			for (int j = 0; j < LastLayer.NeuronCount; j++)
			{
				//get the error value for last layer
				float error = CostFunctionDerivitive(LastLayer.Activations[j], data.activations[j]) * ActivationFunctionDerivitive(LastLayer.WeightedInputs[j]);
				//add value to total values
				localErrors[layers.Length - 1][j] = error;
				errors[layers.Length - 1][j] += error;
				//get the rate of change of cost with respect to the weight between the jth nueron in this layer and the kth neuron in the last layer
				for (int k = 0; k < LastLayer.InputCount; k++)
				{
					float[] prevActivations = layers.Length - 2 < 0 ? data.input : layers[layers.Length - 2].Activations;
					float weightGrad = error * prevActivations[k];
					weightGradients[layers.Length - 1][j, k] += weightGrad;
				}
			}

			//now propagate backwards and get all error values
			for (int l = layers.Length - 2; l >= 0; l--)
			{
				//there is an error for every neuron
				for (int j = 0; j < layers[l].NeuronCount; j++)
				{
					// \/ should be equal to componentwise version of (Transpose(LastLayer.Weights) * error)
					float tWxE = 0; 
					for (int h = 0; h < layers[l + 1].NeuronCount; h++)
						tWxE += layers[l + 1].Weights[h, j] * localErrors[l + 1][h];

					float error = tWxE * ActivationFunctionDerivitive(layers[l].WeightedInputs[j]);
					localErrors[l][j] = error;
					errors[l][j] += error;

					//again, get the rate of change of cost with respect to the weight between the jth nueron in this layer and the kth neuron in the last layer
					for (int k = 0; k < layers[l].InputCount; k++)
					{
						float[] prevActivations = l - 1 < 0 ? data.input : layers[l - 1].Activations;
						float weightGrad = error * prevActivations[k];
						weightGradients[l][j, k] += weightGrad;
					}
				}
			}
		}

		//now should have a gradient that we can use to minimize the cost function
		float gradientMultiplier = learningRate / batchSize;
		//cost needs to be averaged then applied to all weights and biases
		for (int i = 0; i < layers.Length; i++)
		{
			int weightCount = layers[i].InputCount;

			for (int n = 0; n < layers[i].NeuronCount; n++)
			{
				//apply change
				layers[i].Biases[n] -= gradientMultiplier * errors[i][n];

				for (int w = 0; w < weightCount; w++)
				{
					//apply change based on gradient
					layers[i].Weights[n,w] -= gradientMultiplier * weightGradients[i][n, w];
				}
			}
		}
		//average the cost by dividing by number of training datas * outputs


		//note: if any component of the gradient is negative, something is going wrong. if all of them are negative, its probably being inverted somewhere
		//one thing that can cause this is too high a learningRate value in a previous stochastic step
	}

	public static float ActivationFunction(float weightedInput)
	{
		//sigmoid function
		return 1.0f / (1 + Mathf.Exp(-weightedInput));
	}

	public static float ActivationFunctionDerivitive(float weightedInput)
	{
		//sigmoid derivitive (with respect to weightedInput, obviously)
		float sigmoid = ActivationFunction(weightedInput);
		return sigmoid * (1 - sigmoid);
	}

	public static float CostFunction(float activation, float targetActivation)
	{
		//super basic cost function
		float diff = activation - targetActivation;
		return diff * diff;
	}

	public static float CostFunctionDerivitive(float activation, float targetActivation)
	{
		//derivitive with respect to activation value
		return 2 * (activation - targetActivation);
	}

	public struct CorrectInputOutputPair
	{
		public float[] input;
		public float[] activations;
	}
}

public class Layer
{
	float[,] weights;
	float[] biases;
	float[] lastWeightedInputs; //idk pretty sure I need this for backpropagation
	float[] lastActivations;

	public float[] WeightedInputs => lastWeightedInputs;
	public float[] Activations => lastActivations;
	public float[] Biases => biases;
	public float[,] Weights => weights;
	public int NeuronCount { get; private set; }
	public int InputCount { get; private set; }

	public Layer(int neuronCount, int inputCount)
	{
		InputCount = inputCount;
		NeuronCount = neuronCount;
		biases = new float[neuronCount];
		weights = new float[neuronCount, inputCount];

		for (int n = 0; n < neuronCount; n++)
		{
			for (int w = 0; w < inputCount; w++)
			{
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				//			RANDOM.VALUE IS PROBABLY A BAD METHOD 
				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				weights[n, w] = UnityEngine.Random.Range(-1.0f, 1.0f);
			}
			biases[n] = UnityEngine.Random.Range(-1.0f, 1.0f);
		}

		lastWeightedInputs = new float[neuronCount];
		lastActivations = new float[neuronCount];
	}

	public void Evaluate(float[] inputs)
	{
		for (int n = 0; n < NeuronCount; n++)
		{
			float output = biases[n];
			for (int w = 0; w < InputCount; w++)
			{
				output += inputs[w] * weights[n,w];
			}
			lastWeightedInputs[n] = output;
			lastActivations[n] = Network.ActivationFunction(output);
		}
	}
}
