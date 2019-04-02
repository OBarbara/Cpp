/* The game logic (no view code or direct user interaction).
The game is a simple guess the word game based on Mastermind.
*/

#pragma once
#include <string>

// to make the syntax Unreal-friendly
using FString = std::string;
using int32 = int;

struct FBullCowCount
{
	int32 Bulls = 0;
	int32 Cows = 0;
};

enum class EGuessStatus
{
	Invalid_Status,
	OK,
	Not_Isogram,
	Wrong_Length,
	Not_Lowercase
};

enum class EResetStatus
{
	OK,
	No_Hidden_Word
};

class FBullCowGame
{
public:
	FBullCowGame();	// constructor

	int32 GetMaxTries() const;
	int32 GetCurrentTry() const;
	int32 GetHiddenWordLength() const;
	bool IsGameWon() const;

	EGuessStatus CheckGuessValidity(FString) const;
	void Reset();
	void SetHiddenWordLength(int32);
	FBullCowCount SubmitValidGuess(FString);

private:
	int32 MyMaxTries;
	int32 MyCurrentTry;
	FString MyHiddenWord;
	bool bGameWon;

	bool IsIsogram(FString) const;
	bool IsLowercase(FString) const;
};