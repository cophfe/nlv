#include "NetworkEvolver.h"
#include <crtdbg.h>
#include "Application.h"

char* Invert(char* string)
{
	//switch the first word with the last word, then switch the second word with the second last word, etc
	
	//length will point to the '\0' or ' ' at the end of the second word being switched
	int length = 0;
	while (string[length] != '\0')
		length++;

	//get number of words
	int words = 1;
	for (size_t i = 0; i < length; i++)
	{
		if (string[i] == ' ')
			words++;
	}

	//end pointer will point to the start of the second word being switched
	int endPointer;
	//start pointer will point to the start of the first word being switched 
	int startPointer = 0;
	while (words >= 2)
	{
		endPointer = length - 1;

		//get last word pointer
		for (; endPointer >= 0; endPointer--)
		{
			if (string[endPointer] == ' ')
				break;
		}
		endPointer++;

		//copy pointers in order to iterate through the words
		int lastWordPointer = endPointer;
		int startWordPointer = startPointer;
		int startWordSize = 0;
		int endWordSize = length - lastWordPointer;

		while (string[startWordPointer] != ' ')
		{
			//switch letters in first and last words
			char cache = string[lastWordPointer];
			string[lastWordPointer] = string[startWordPointer];
			string[startWordPointer] = cache;

			startWordSize++;
			startWordPointer++;
			lastWordPointer++;

			//if reached the end of the last word
			if (lastWordPointer == length)
			{
				//while there is still first word to be switched with second word
				while (string[startWordPointer] != ' ')
				{
					//move the entire string backwards and insert letter at last position
					char cache = string[startWordPointer];
					int pointer = startWordPointer;
					while (pointer + 1 < length)
					{
						string[pointer] = string[pointer + 1];
						pointer++;
					}
					string[lastWordPointer - 1] = cache;

					startWordSize++;
				}

				break;
			}
		}

		int savedStartPointer = startWordPointer;
		int savedLastWordPointer = lastWordPointer;
		//if there is still second word to be switched with first word
		while (savedLastWordPointer < length)
		{
			//get the letter from the second word be put at the end of the first word
			char cache = string[savedLastWordPointer];
			//remove this letter from the second word
			int pointer = savedLastWordPointer;
			while (pointer + 1 < length)
			{
				string[pointer] = string[pointer + 1];
				pointer++;
			}
			pointer = savedStartPointer;
			//move the entire string forwards and insert letter at end of first word
			while (pointer < length)
			{
				char cache2 = string[pointer];
				string[pointer] = cache;
				cache = cache2;
				pointer++;
			}
			savedLastWordPointer++;
			savedStartPointer++;
		}

		//two words have been switched
		words -= 2;
		length -= startWordSize + 1;
		startPointer += endWordSize + 1;

	}
	return string;
}

void main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//reverse word order in string, without reversing letter order inside of words, without creating an additional array
	//char string[] = "a sad monkey cries a lot which in turn makes me feel a little sadder than is usual";
	//Invert(string);
	//std::cout << string;
	
	Application game;
	game.Run();
}