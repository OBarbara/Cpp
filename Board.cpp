#include "Board.h"

void Board::newGame() {
	this->~Board();	// discard current board
	Board();		// create a new one with the board filled with nulls
	bool is_white = true;	// start from white pieces for rows of squares 8, 7 and 6
	for (int i = 0; i < SIZE; i++) {
		if (i == 3) {
			is_white = false;	// change to false to represent red pieces
			i = i + 2;
		}
		for (int j = 0; j < SIZE; j++) {
			if ((i + j) % 2 == 0) {
				m_board[i][j] = new Man(is_white);	// create a new man draught and place it on the board (dynamically allocated)
			}
		}
	}
	// generate a number between 0 and 1 to determine which player starts first
	srand((unsigned int) time(NULL));	// C-style type cast to unsigned int to avoid data loss warning
	m_turn = (rand() % 2) + 1;
	std::cout << "Player " << m_turn << " starts first." << std::endl;
	displayBoard();
}

void Board::displayBoard() const{
	system("cls");
	std::cout << "\nPlayer " << m_turn << "'s turn.\n" << std::endl;
	std::cout << "\n\t   Player 2 <White>\n\n\t   A B C D E F G H" << std::endl;
	for (int i = 0; i < SIZE; i++) {	// rows
		std::cout << "\t" << SIZE - i << "  ";
		for (int j = 0; j < SIZE; j++) {	// columns
			// display the current row
			if ((i + j) % 2 != 0)
				std::cout << ". ";		// for invalid squares
			else if (m_board[i][j] == nullptr)
				std::cout << "  ";		// for valid but empty squares
			else
				std::cout << m_board[i][j]->getSymbol() << " ";	// get the symbol of the piece occupying the non-vacant cell
		}
		std::cout << "  " << SIZE - i << std::endl;
	}
	std::cout << "\n\t   A B C D E F G H\n\n\t   Player 1 <Red>" << std::endl;
}


Draught* Board::getPieceAt(int row, int column) const {
	if (row > 7 || row < 0 || column < 0 || column > 7)
		return nullptr;		// return a null pointer if a piece is out of range
	else
		return m_board[row][column];	// return the pointer to the piece at the given coordinates
}



Board::~Board() {
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (m_board[i][j] != nullptr) {
				delete m_board[i][j];	// delete memory where piece resides if the board square is not empty
				m_board[i][j] = nullptr;
			}
		}
	}
}