using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;
using UnityEngine.UI;
using MNIST.IO;
using System.IO.Compression;
using System.IO;

public class Trainer : MonoBehaviour
{
	public TextMeshProUGUI totalEpochsText;
	public TextMeshProUGUI accuracyText;
	public TextMeshProUGUI improvementText;
	public TextMeshProUGUI epochText;
	public TextMeshProUGUI batchText;
	public TextMeshProUGUI learningText;
	public Button resetButton;
	public Button trainButton;
	public Slider epochSlider;
	public Slider batchSlider;
	public Slider learningRateSlider;
	public string subFolder;
	Network network;
	int epochs = 1;
	float learningRate = 3.0f;
	int batchSize = 10;
	float lastAccuracy = 0.0f;
	int totalEpochs = 0;

	Network.CorrectInputOutputPair[] trainingPairs;
	TestInput[] testingPairs;
	struct TestInput
	{
		public float[] input;
		public byte correctNumber;
	}

	void Start()
    {

		Debug.Log("<color=red>Loading mnist data...</color>");

		string folder = Application.dataPath + "/" + subFolder;

		//uses https://github.com/guylangston/MNIST.IO
		//I made slightly modified functions but they still use things from the nuget package in those functions
		var images = LoadImageFiles($"{folder}/train-images-idx3-ubyte.gz");
		var labels = LoadLabelFiles($"{folder}/train-labels-idx1-ubyte.gz");

		trainingPairs = new Network.CorrectInputOutputPair[50000];

		for (int i = 0; i < trainingPairs.Length; i++)
		{
			trainingPairs[i].activations = new float[10];
			trainingPairs[i].activations[labels[i]] = 1;
			trainingPairs[i].input = images[i];
		}

		testingPairs = new TestInput[10000];
		for (int i = 0; i < testingPairs.Length; i++)
		{
			testingPairs[i].correctNumber = labels[trainingPairs.Length + i];
			testingPairs[i].input = images[trainingPairs.Length + i];
		}
		network = new Network(784, 16, 16, 10);

		Debug.Log("<color=green>Finished loading mnist data.</color>");

		epochSlider.onValueChanged.AddListener(ChangeEpochs);
		batchSlider.onValueChanged.AddListener(ChangeBatchSize);
		learningRateSlider.onValueChanged.AddListener(ChangeLearningRate);
		resetButton.onClick.AddListener(OnReset);
		trainButton.onClick.AddListener(Train);
		ChangeEpochs(epochSlider.value);
		ChangeBatchSize(batchSlider.value);
		ChangeLearningRate(learningRateSlider.value);

		totalEpochsText.text = $"Total elapsed epochs: 0";
		Test();
	}

    public void Train()
	{
		Debug.Log("<color=red>Started training...</color>");
		for (int i = 0; i < epochs; i++)
		{
			network.Train(batchSize, trainingPairs, learningRate);
			Debug.Log("<color=orange>Finished epoch #" + totalEpochs + "</color>");
			totalEpochs++;
		}
		totalEpochsText.text = $"Total elapsed epochs: {totalEpochs}";
		Debug.Log("<color=green>Finished training.</color>");
		Test();
	}

	void Test()
	{
		Debug.Log("<color=red>Started testing...</color>");
		int correctCount = 0;
		for (int i = 0; i < testingPairs.Length; i++)
		{
			float[] output = network.Evaluate(testingPairs[i].input);

			float highestActivation = 0;
			int index = 0;
			for (int j = 0; j < 10; j++)
			{
				if (output[j] > highestActivation)
				{
					highestActivation = output[j];
					index = j;
				}
			}
			if (index == testingPairs[i].correctNumber)
				correctCount++;
		}

		float accuracy = (float)correctCount / testingPairs.Length;
		accuracyText.text = "Current Accuracy: " + (100 * accuracy).ToString("0.00") + "%";
		improvementText.text = "Improvement: +" + (100 * (accuracy - lastAccuracy)).ToString("0.00") + "%";
		
		lastAccuracy = accuracy;
		Debug.Log("<color=green>Finished testing.</color>");
	}

	void ChangeEpochs(float value)
	{
		epochs = (int)value;
		epochText.text = $"Train for {epochs} epochs";
	}

	void ChangeBatchSize(float value)
	{
		int val = trainingPairs.Length / (int)(trainingPairs.Length / value);
		batchSize = val;
		batchText.text = $"Batch size of {batchSize}";
	}

	void ChangeLearningRate(float value)
	{
		learningRate = value;
		learningText.text = $"Learning rate of {learningRate}";
	}

	private void OnReset()
	{
		lastAccuracy = 0;
		network = new Network(784, 16, 16, 10);
		totalEpochsText.text = $"Total elapsed epochs: 0";
		Test();
	}


	byte[] LoadLabelFiles(string labelFile)
	{
		using (FileStream stream = File.OpenRead(labelFile))
		{
			using (GZipStream stream2 = new GZipStream(stream, CompressionMode.Decompress))
			{
				using (BinaryReaderMSB binaryReaderMSB = new BinaryReaderMSB(stream2))
				{
					int num = binaryReaderMSB.ReadInt32MSB();
					if (num != 2049)
					{
						throw new InvalidDataException(num.ToString("x"));
					}

					int count = binaryReaderMSB.ReadInt32MSB();
					return binaryReaderMSB.ReadBytes(count);
				}
			}
		}
	}

	float[][] LoadImageFiles(string imageFile)
	{
		using (FileStream raw = File.OpenRead(imageFile))
		{
			using (GZipStream gz = new GZipStream(raw, CompressionMode.Decompress))
			{
				using (BinaryReaderMSB reader = new BinaryReaderMSB(gz))
				{
					int num = reader.ReadInt32MSB();
					if (num != 2051)
					{
						throw new InvalidDataException(num.ToString("x"));
					}

					int itemCount = reader.ReadInt32MSB();
					int rowCount = reader.ReadInt32MSB();
					int colCount = reader.ReadInt32MSB();
					float[][] images = new float[itemCount][];

					for (int i = 0; i < itemCount; i++)
					{
						images[i] = new float[rowCount * colCount];
						for (int j = 0; j < rowCount * colCount; j++)
						{
							images[i][j] = (float)reader.ReadByte()/255.0f;
						}
					}
					return images;
				}
			}
		}
	}

}
