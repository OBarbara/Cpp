#include "BoardCell.h"

int BoardCell::getRow() const{
	int  row = 56 - m_coords.at(1);	// 56 is the ASCII decimal equivalent to the number of columns/rows in the board array (character 8)
	if (row < 0 || row > 7)
		throw OutOfBoardException();
	else 
		return row;
}

int BoardCell::getColumn() const {
	char ch = m_coords.at(0);	// the character ch represents the column of the board
	int col = (isupper(ch)) ? (int) (ch - 65) : (int) (ch - 97);	// return the integer equivalent to that column
	if (col < 0 || col > 7)
		throw OutOfBoardException();
	else 
		return col;
}
