#pragma once
#include "Board.h"
#define FILESUFFIX ".txt"
#define PATH "D:\\Reports and Assignments\\Year 2\\Sem 1\\C++ Workspace\\Assignment\\Assignment\\Saved Games\\"
class FileHandler
{
public:
	void saveGame(Board& board) const;	// saves the current game
	bool loadGame(Board& board);	// loads a saved game from the Saved Games folder
};

