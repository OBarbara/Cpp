#pragma once
#include <iostream>
#include <string>
#include <time.h>	// used for srand() function to randomly select which player starts first with rand()
#include <fstream>	// for saving and loading games from files
#include "BoardCell.h"
#include "OutOfBoardException.h"
#include "Man.h"
#include "King.h"
#define SIZE 8
class Board
{
private:
	Draught* m_board[SIZE][SIZE];	// contains the board filled with checkers piece pointers
	unsigned short m_turn = 0;	// determines which player is currently taking his turn
public:
	Board()
		: m_board() {}	// initialize the board to all null pointers

	unsigned short getTurn() const { return m_turn; }
	void setTurn(unsigned short turn) { m_turn = turn; }
	void newGame();	// discard current game (if any) and create new game
	void displayBoard() const;	// display the board
	void setPieceAt(int row, int column, Draught* piece = nullptr) { m_board[row][column] = piece; }
	Draught* getPieceAt(int row, int column) const;	// get piece at the coordinates specified
	Draught* getPieceAt(BoardCell& square) const { return m_board[square.getRow()][square.getColumn()]; }	// get piece at the square specified

	~Board();	// destroy the board and all de-allocate memory pointed to by all the piece pointers present in the board array
};