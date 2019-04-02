#pragma once
#include "FBullCowGame.h"
#include <map>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#define TMap std::map

FBullCowGame::FBullCowGame()	// default constructor
{
	Reset();
}

int32 FBullCowGame::GetCurrentTry() const { return MyCurrentTry; }

int32 FBullCowGame::GetHiddenWordLength() const{ return MyHiddenWord.length(); }

bool FBullCowGame::IsGameWon() const { return bGameWon; }

int32 FBullCowGame::GetMaxTries() const
{
	TMap<int32, int32> WordLengthToMaxTries{ {3, 3}, {4, 5}, {5, 7
	}, {6, 9}, {7, 12} };
	return WordLengthToMaxTries[MyHiddenWord.length()];
}

bool FBullCowGame::IsIsogram(FString Word) const
{
	// treat 0 and 1 letter words as isograms
	if (Word.length() <= 1) { return true; }

	TMap<char, bool> LetterSeen;	// setup our map
	for (auto Letter : Word)		// for all letters of the word
	{
		Letter = tolower(Letter);	// handle mixed case
		// if the letter is not in the map, add it to the map and keep going
		if (LetterSeen[Letter]) { 
			return false; 
		}
		else { 
			LetterSeen[Letter] = true;
		}
	}
	return true;	// for example in cases where \0 is entered
}

bool FBullCowGame::IsLowercase(FString Word) const
{
	for (auto Letter : Word)		// for all letters of the word
	{
		if (!islower(Letter)) {
			return false;
		}
	}
	return true;	// for example in cases where \0 is entered
}

void FBullCowGame::Reset()
{
	MyCurrentTry = 1;
	bGameWon = false;
	return;
}

void FBullCowGame::SetHiddenWordLength(int32 len)
{
	/* initialize random seed: */
	srand(time(NULL));
	// generate a random integer in the range from 1 to 4
	int selection = rand() % 4 + 1;
	switch (len) {
		case 3:
			switch (selection) {
				case 1:
					MyHiddenWord = "pet";
					break;
				case 2: 
					MyHiddenWord = "car";
					break;
				case 3:
					MyHiddenWord = "fan";
					break;
				case 4:
					MyHiddenWord = "ear";
					break;
				default:
					std::cerr << "Error in selecting word." << std::endl;
					return;
			}
			break;
		case 4:
			switch (selection) {
				case 1:
					MyHiddenWord = "pond";
					break;
				case 2:
					MyHiddenWord = "rack";
					break;
				case 3:
					MyHiddenWord = "land";
					break;
				case 4:
					MyHiddenWord = "trip";
					break;
				default:
					std::cerr << "Error in selecting word." << std::endl;
					return;
			}
			break;
		case 5:
			switch (selection) {
				case 1:
					MyHiddenWord = "break";
					break;
				case 2:
					MyHiddenWord = "eagle";
					break;
				case 3:
					MyHiddenWord = "quote";
					break;
				case 4:
					MyHiddenWord = "fries";
					break;
				default:
					std::cerr << "Error in selecting word." << std::endl;
					return;
			}
			break;
		case 6:
			switch (selection) {
				case 1:
					MyHiddenWord = "fridge";
					break;
				case 2:
					MyHiddenWord = "mantle";
					break;
				case 3:
					MyHiddenWord = "zealot";
					break;
				case 4:
					MyHiddenWord = "branch";
					break;
				default:
					std::cerr << "Error in selecting word." << std::endl;
					return;
			}
		case 7:
			switch (selection) {
			case 1:
				MyHiddenWord = "pension";
				break;
			case 2:
				MyHiddenWord = "bouncer";
				break;
			case 3:
				MyHiddenWord = "special";
				break;
			case 4:
				MyHiddenWord = "pathing";
				break;
			default:
				std::cerr << "Error in selecting word." << std::endl;
				return;
			}
			break;
		default:
			std::cerr << "Invalid word length." << std::endl;
			return;
	}
}


EGuessStatus FBullCowGame::CheckGuessValidity(FString Guess) const
{
	if (!IsIsogram(Guess)) // if the guess isn't an isogram
	{
		return EGuessStatus::Not_Isogram;
	}
	else if (!IsLowercase(Guess)) // if the guess isn't all lowercase
	{
		return EGuessStatus::Not_Lowercase;	// TODO write function
	}
	else if(GetHiddenWordLength() != Guess.length()) // if the guess length is wrong
	{ 
		return EGuessStatus::Wrong_Length;
	}
	else // otherwise
	{
		return EGuessStatus::OK;
	}
}

// receives a VALID guess, increments try, and returns count
FBullCowCount FBullCowGame::SubmitValidGuess(FString Guess)
{
	MyCurrentTry++;
	FBullCowCount BCCount;
	
	// loop through all letters in the hidden word
	int32 WordLength = MyHiddenWord.length();	// assuming same length as guess
	for (int32 MHWChar = 0; MHWChar < WordLength; MHWChar++) {
		// compare letters against the guess
		for (int32 GChar = 0; GChar < WordLength; GChar++) 
		{
			// if they're in the same place and they match
			if (Guess[MHWChar] == MyHiddenWord[GChar]) {
				if (MHWChar == GChar) {
					BCCount.Bulls++;
				}
				else {
					BCCount.Cows++;
				}
			}
		}
	}
	if (BCCount.Bulls == WordLength) {
		bGameWon = true;
	}
	else {
		bGameWon = false;
	}
	return BCCount;
}
