using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;
using UnityEngine.UI;
using MNIST.IO;
using System.IO.Compression;
using System.IO;
using System.Threading;
using UnityEngine.EventSystems;

public class Trainer : MonoBehaviour
{
	public TextMeshProUGUI resultText;
	public TextMeshProUGUI valueText;
	public TextMeshProUGUI percentageResultsText;
	public RawImage previewImage;
	public Button randomTest;
	public GameObject trainingPanel;
	
	[Space(4)]
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
	float currentAccuracy = 0.0f;
	int totalEpochs = 0;
	bool running;
	Network.CorrectInputOutputPair[] trainingPairs;
	TestInput[] testingPairs;

	Texture2D previewTexture;
	
	struct TestInput
	{
		public float[] input;
		public byte correctNumber;
	}

	void Start()
    {
		previewTexture = new Texture2D(28, 28, TextureFormat.Alpha8, false);
		previewTexture.filterMode = FilterMode.Point;
		previewImage.texture = previewTexture;
		randomTest.onClick.AddListener(TestPreview);

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
		network = new Network(784, 30, 10);

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
		//running should only be changed by this thread, so no problems
		if (!running)
		{
			running = true;
			Thread thread = new Thread(() => 
			{
				Debug.Log("<color=red>Started training...</color>");
				for (int i = 0; i < epochs; i++)
				{
					network.Train(batchSize, trainingPairs, learningRate);
					Debug.Log("<color=orange>Finished epoch #" + totalEpochs + "</color>");
					Test();
					totalEpochs++;
				}
				Debug.Log("<color=green>Finished training.</color>");
				running = false;
			});
			thread.Start();
		}
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

		lastAccuracy = currentAccuracy;
		currentAccuracy = (float)correctCount / testingPairs.Length;
		
		Debug.Log("<color=green>Finished testing.</color>");
	}

	private void Update()
	{
		totalEpochsText.text = $"Total elapsed epochs: {totalEpochs}";
		accuracyText.text = "Current Accuracy: " + (100 * currentAccuracy).ToString("0.00") + "%";
		improvementText.text = "Improvement: +" + (100 * (currentAccuracy - lastAccuracy)).ToString("0.00") + "%";
		trainingPanel.SetActive(running);
	}

	public void TestPreview()
	{
		if (running)
			return;

		int index = Random.Range(0, testingPairs.Length);

		byte[] alphaData = new byte[testingPairs[index].input.Length];
		for (int i = 0; i < alphaData.Length; i++)
		{
			alphaData[i] = (byte)(255 * testingPairs[index].input[i]);
		}
		previewTexture.SetPixelData(alphaData, 0);
		previewTexture.Apply();

		string percentageResults = "";
		float[] output = network.Evaluate(testingPairs[index].input);
		float highestActivation = 0;
		int highestNumberIndex = 0;
		for (int j = 0; j < 10; j++)
		{
			if (output[j] > highestActivation)
			{
				highestActivation = output[j];
				highestNumberIndex = j;
			}
		}

		for (int j = 0; j < 10; j++)
		{
			if (j == highestNumberIndex)
				percentageResults += $"<b>{j}: {output[j].ToString("0.000")}</b>\n";
			else
				percentageResults += $"{j}: {output[j].ToString("0.000")}\n";
		}

		percentageResultsText.text = percentageResults;
		valueText.text = "Value: " + testingPairs[index].correctNumber;
		resultText.text ="Result: " + highestNumberIndex;
		if (highestNumberIndex == testingPairs[index].correctNumber)
			resultText.color = Color.green;
		else
			resultText.color = Color.red;
	}

	void ChangeEpochs(float value)
	{
		if (running)
			return;

		epochs = (int)value;
		epochText.text = $"Train for {epochs} epochs";
	}

	void ChangeBatchSize(float value)
	{
		if (running)
			return;

		int val = trainingPairs.Length / (int)(trainingPairs.Length / value);
		batchSize = val;
		batchText.text = $"Batch size of {batchSize}";
	}

	void ChangeLearningRate(float value)
	{
		//StartCoroutine(
		//	() => new WaitUntil(() => running)
		//	);
		if (running)
			return;

		learningRate = value;
		learningText.text = $"Learning rate of {learningRate}";
	}

	private void OnReset()
	{
		if (running)
			return;

		lastAccuracy = 0;
		network = new Network(784, 30, 10);
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
