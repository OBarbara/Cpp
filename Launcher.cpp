#include <typeinfo>
#include "GameController.h"
#include "FileHandler.h"
int menu() {
	int choice;
	std::cout << "Would you like to:\n1. Start a new game.\n2. Save the current game.\n3. Load a saved game.\n4. Exit." << std::endl;
	std::cin >> choice;
	return choice;
}

int main() {
	std::cout << "\t---------- Welcome to Draughts ----------\n" << std::endl;
	GameController game;
	Board board;
	FileHandler file;
	bool game_over = false;	// defines whether the game has ended or not
	bool null = true;	// this is true only when a game has not been started yet and the baord is still empty
	std::string move = "";
	int selection = 0;	// initialize selection to 0 to pass loop condition
	while (selection != 4) {
		selection = menu();
		switch (selection) {
		case 1: board.newGame();	// start a new game
			null = false;
			break;
		case 2: if (null) {
			system("cls");	// clear the screen
			std::cout << "Please start a new game or load an existing one before attempting to save.\n" << std::endl;
			continue;
		}
				else
					file.saveGame(board);
			break;
		case 3:
			if (!file.loadGame(board))
				continue;
			null = false;
			break;
		case 4:
			continue;
		default:
			// tell the user that the choice was invalid so that he enters a valid choice from 1-4
			std::cerr << "Invalid choice. Please enter a number from 1 to 4." << std::endl;
			continue;
		}
		std::cout << "Please enter the move coordinates." << std::endl;
		std::cin >> move;
		while (move != "0") {
			if (move.size() == 4 && game.movePiece(board, move)) {
				if (game.checkIfWinner(board)) {
					game_over = true;
					break;	// end the program if there is a winner
				}
				else {
					std::cout << "Please enter the move coordinates." << std::endl;
					std::cin >> move;
				}
			}
			else {
				std::cin >> move;	// user enters the move coordinates
				continue;
			}
		}
		if (game_over) break;	// if there is a winner, the gane has ended
	}
	std::cout << "Game over." << std::endl;
	return 0;	// program terminated
}