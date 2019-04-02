/* This is the console executable that makes used of the BullCow class
This acts as the view in a MVC pattern, and is responsible for all user
interaction. For game logic see the FBullCowGame class.
*/

#include <iostream>
#include <string>
#include <limits>
#include "FBullCowGame.h"
// always include verbs in function names
// function names should always describe their behavior as a whole

// to make syntax Unreal-friendly
using FText = std::string;	// FText generally used for user output
using int32 = int;

// function prototypes as outside a class
void PrintIntro();
void PlayGame();
FText GetValidGuess();
bool AskToPlayAgain();
void PrintGameSummary();
void SetDifficulty();

FBullCowGame BCGame;	// instantiate a new game, which we re-use across plays

int main()
{
	do
	{
		PrintIntro();
		PlayGame();
		std::cout << std::endl;
	} while (AskToPlayAgain());
	return 0;
}

// plays a single game to completion
void PlayGame()
{
	BCGame.Reset();
	int32 MaxTries = BCGame.GetMaxTries();

	// loop asking for guesses while the game 
	// is NOT won and there are still tries remaining
	while (!BCGame.IsGameWon() && BCGame.GetCurrentTry() <= MaxTries) {
		FText Guess = GetValidGuess();	// TODO make loop checking valid
		
		// submit valid guess to the game
		FBullCowCount BullCowCount = BCGame.SubmitValidGuess(Guess);

		// print number of bulls and cows
		std::cout << "Bulls = " << BullCowCount.Bulls;
		std::cout << "\tCows = " << BullCowCount.Cows << std::endl;

		std::cout << std::endl;
	}
	PrintGameSummary();
	return;
}

void PrintIntro()
{
	std::cout << "Welcome to Bulls and Cows, a fun word game.\n";
	std::cout << std::endl;
	std::cout << "          }   {         ___ " << std::endl;
	std::cout << "          (o o)        (o o) " << std::endl;
	std::cout << "   /-------\\ /          \\ /-------\\ " << std::endl;
	std::cout << "  / | BULL |O            O| COW  | \\ " << std::endl;
	std::cout << " *  |-,--- |              |------|  * " << std::endl;
	std::cout << "    ^      ^              ^      ^ \n" << std::endl;
	SetDifficulty();
	std::cout << "Can you guess the " << BCGame.GetHiddenWordLength(); 
	std::cout << " letter isogram I'm thinking of?\n";
	return;
}

// get a guess from the player
FText GetValidGuess()
{
	FText Guess = "";	// variable names begin with a capital letter to accomodate Unreal format
	EGuessStatus Status = EGuessStatus::Invalid_Status;
	do {
		int32 CurrentTry = BCGame.GetCurrentTry();
		std::cout << "Try " << CurrentTry << " of " << BCGame.GetMaxTries();
		std::cout << ". Enter your guess: ";
		getline(std::cin, Guess);

		Status = BCGame.CheckGuessValidity(Guess);

		switch (Status)
		{
		case EGuessStatus::Wrong_Length:
			std::cout << "Please enter a " << BCGame.GetHiddenWordLength() << " letter word.\n\n";
			break;
		case EGuessStatus::Not_Isogram:
			std::cout << "Please enter a word without repeating characters.\n\n";
			break;
		case EGuessStatus::Not_Lowercase:
			std::cout << "The isogram must be in lowercase.\n\n";
			break;
		default:	// assume the guess is valid
			break;
		}
	} while (Status != EGuessStatus::OK);	// keep looping until we get no errors
	return Guess;
}
void SetDifficulty(){
	int wordlen = 0;
	std::cout << "How long do you wish the word to be (3-7 letters)?" << std::endl;
	std::cin >> wordlen;
	while (wordlen < 3 || wordlen > 7) {
		std::cout << "The length must be between 3 and 7 (inclusive)." << std::endl;
		std::cin >> wordlen;
	}
	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	BCGame.SetHiddenWordLength(wordlen);
}


bool AskToPlayAgain()
{
	std::cout << "Do you want to play again (y/n) ? \n" << std::endl;
	FText Response = "";
	getline(std::cin, Response);

	return (Response[0] == 'y') || (Response[0] == 'Y');
}

void PrintGameSummary()
{
	if (BCGame.IsGameWon()) {
		std::cout << "Well Done! You won.\n";
	}
	else {
		std::cout << "Better luck next time!\n";
	}
	return;
}
