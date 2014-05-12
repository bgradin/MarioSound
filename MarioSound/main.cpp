#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "SoundEffect.h"
using namespace std;

int main(char argc, char** argv)
{
	if (argc != 2)
	{
		cout << "Invalid number of parameters" << endl;
		return 0;
	}

	if (strcmp(argv[1], "/?") == 0)
	{
		cout << "This program takes in a file with sound data as a parameter and allows the sounds to be played through the system beep mechanism." << endl;
		return 0;
	}

	ifstream in(argv[1]);
	map<string, SoundEffect> soundEffects;

	while (in.peek() != EOF)
	{
		// Get name of sound effect
		string name;
		in >> name;

		// Find out how long the sound effect note location array should be
		int position = (int) in.tellg(), numbers = 0;
		char c;
		while (in.peek() != '\n' && in.peek() != EOF)
		{
			if ((c = in.get()) == ' ' || c == '\t')
				numbers++;
		}
		in.seekg(position);

		// Declare note location array
		int* frequencyArray = new int[numbers];

		// Read note locations
		for (int i = 0; i < numbers; i++)
			in >> frequencyArray[i];

		// Create sound effect
		SoundEffect* newSoundEffect = new SoundEffect(frequencyArray, numbers);

		if (soundEffects.find(name) == soundEffects.end())
			soundEffects[name] = (*newSoundEffect);

		// Clean up memory
		delete [] frequencyArray;
		delete newSoundEffect;

		// Discard newline character
		in.get();
	}

	in.close();
	string bla = "";

	do {
		if (bla == "list")
		{
			cout << endl;

			for (map<string, SoundEffect>::const_iterator cur = soundEffects.begin(); cur != soundEffects.end(); cur++)
				cout << (*cur).first << endl;

			cout << endl;
		}
		if (bla == "cls")
			system("cls");

		cin >> bla;

		if (soundEffects.find(bla) != soundEffects.end())
			soundEffects[bla].Play();
	} while (bla != "exit");
}