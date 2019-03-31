#include "FileHandler.h"



void FileHandler::saveGame(Board& board) const {
	// "name" holds the name of the file which the user specifies for the current game
	// colour represents the current player taking his turn
	std::string name;
	std::string colour = (board.getTurn() == 1) ? "Red" : "White";	// determines which colour has the current turn
	std::cout << "Please enter the name of the file you desire your game to be stored in." << std::endl;
	std::cin >> name;		// user enters the name of the file
	std::ofstream saveFile(PATH + name + FILESUFFIX);	// open the file for writing
	saveFile << "Next: " << colour << std::endl;	// display turn
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (board.getPieceAt(i, j) != nullptr && saveFile)	// check whether saveFile is still open and if the current coordinates point to null
														// note that the + 97 is the offset for the ascii table to represent the small letters in the coordinates which are written in the file
				saveFile << (board.getPieceAt(i, j))->getSymbol() << ":" << (char)(j + 97) << SIZE - i << std::endl;
			else if (!saveFile) {
				std::cerr << "Error while handling file." << std::endl;	// display error if file was not opened correctly
				return;		// exit function if file was not opened correctly
			}
		}
	}
	board.displayBoard();
}

bool FileHandler::loadGame(Board& board) {
	Draught* piece;
	board.~Board();
	std::string name, line;	// name holds the name of the file, and line is a buffer for each line taken from the load file
		std::cout << "Please enter the name of the file you want to load." << std::endl;
		std::cin >> name;
		std::ifstream gameFile(PATH + name + FILESUFFIX);	// open gameFile for reading
		if (gameFile) {
			getline(gameFile, line);
			// if the file states that red is next, the turn is given to player 1 and similarly for white
			// the following takes the name of the colour from the first line of the file
			line.substr(line.find(" ") + 1) == "Red" ? board.setTurn(1) : board.setTurn(2);
			while (!gameFile.eof()) {
				getline(gameFile, line);	// get a new line from the game file and store it in the "line" variable
				if (line.empty()) continue;	// when end of file is reached, line will be an empty string
				BoardCell square(line.substr(line.find(":") + 1));
				char symbol = line.at(0);
				// define the piece by its symbol
				if (symbol == 'o')
					piece = new Man(false);
				else if (symbol == 'x')
					piece = new Man(true);
				else if (symbol == '*')
					piece = new King(false);
				else if (symbol == '#')
					piece = new King(true);
				else
					throw BoardException();	// throw unknown base exception

				board.setPieceAt(square.getRow(), square.getColumn(), piece);	// set the piece at the current row and column to the appropriate piece
			}
			std::cout << "Loaded game data." << std::endl;
			board.displayBoard();	// display the loaded board
			return true;
		}

		else {
			system("cls");	// clear the screen
			std::cerr << "Error while loading file." << std::endl;
			return false;	// return unsuccessul operation
		}
}