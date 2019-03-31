#pragma once
#include "Board.h"
#include "BoardException.h"
#include "InvalidPieceException.h"
#include "InvalidTypeException.h"
#include "InvalidMoveException.h"
class GameController
{
public:
	bool movePiece(Board& board, std::string move);	// takes the move coordinates to move a piece
	bool jumpPiece(Board& board, Draught* from, int fromRow, int fromCol, Draught* to, int toRow, int toCol);	// used to capture an opponent's piece
	bool possCapture(Board& board, Draught* from, int x, int y) const;	// check whether it is possible to capture an opponent's piece
	bool checkIfWinner(Board& board);	// check if there is a winner after a move

};

