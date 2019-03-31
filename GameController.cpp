#include "GameController.h"

bool GameController::movePiece(Board& board, std::string move) {
	bool moved = false;
	std::string src_coords = move.substr(0, 2);	// string representation of the piece needed to be moved
	std::string dest_coords = move.substr(2, 2);	// string representation of the square to which the piece is to be moved
													// now translate the string coordinates into board square coordinates that can be handled by the array
	BoardCell fromIndex(src_coords);
	BoardCell toIndex(dest_coords);
	try {

		Draught* from = board.getPieceAt(fromIndex);	// holds piece at from coordinate
		Draught* to = board.getPieceAt(toIndex);	// holds piece at to coordinate

											// set cell rows and columns as concise variable names
		int fRow = fromIndex.getRow();	// row where the piece to be moved originates
		int fCol = fromIndex.getColumn();	// column where the piece to be moved originates
		int tRow = toIndex.getRow();	// the destination row
		int tCol = toIndex.getColumn();	// the destination column

										// check whether there is a piece present at the coordinates FROM and that there are no pieces present at the TO coordinates;
										// also check whether the player is picking his own piece up to be moved and not the opponent's one
		if (from != nullptr && to == nullptr &&
			((!from->isWhite() && board.getTurn() == 1) || (from->isWhite() && board.getTurn() == 2))) {
			if ((tCol == fCol + 1 || tCol == fCol - 1) && (tRow == fRow + 1 || tRow == fRow - 1)) {	// check whether it is a move command or a jump command
				if ((from->isWhite() && !from->isKing() && tRow == fRow + 1) ||			// check whether move command is valid for the particular colour and piece type
					(!from->isWhite() && !from->isKing() && tRow == fRow - 1) ||
					from->isKing()) {
					board.setPieceAt(tRow, tCol, from);	// move the source piece to the destination square
					to = from;
					board.setPieceAt(fRow, fCol);	// set the source square to point to null
					moved = true;	// set moved to true to indicate success in moving
					if (to->canKing(tRow) && !to->isKing()) {	// if a man piece reaches the opposite end of the board it gets promoted to a king with the same colour
						bool white = to->isWhite();
						delete to;
						Draught* pking = new King(white);	// a new king pointer is created at the same location as the man before it with the same colour
						board.setPieceAt(tRow, tCol, pking);
					}
				}

				else {
					throw InvalidTypeException(from->isKing());
				}
			}

			else if (possCapture(board, from, fRow, fCol) && (tCol == fCol + 2 || tCol == fCol - 2) && (tRow == fRow + 2 || tRow == fRow - 2)) {	// this checks whether it is a jump
				moved = jumpPiece(board, from, fRow, fCol, to, tRow, tCol);	// jump a piece
			}

			else {
				throw InvalidMoveException();
			}
		}
		else {
			throw InvalidPieceException();
		}
	}
	catch (InvalidTypeException& e) {	// piece type does not match with the move command
		std::cerr << e.getMessage() << std::endl;
	}
	catch (OutOfBoardException& e) {		// when trying to access a cell which is out of the board
		std::cerr << e.getMessage() << std::endl;
	}
	catch (InvalidPieceException& e) {	// for when trying to access a wrong piece
		std::cerr << e.getMessage() << std::endl;
	}
	catch (InvalidMoveException& e) {	// for invalid move coordinates
		std::cerr << e.getMessage() << std::endl;
	}
	catch (BoardException& e) {	// base exception
		std::cerr << e.getMessage() << std::endl;
	}
	return moved;	// return whether or not the move command was successful
}


bool GameController::jumpPiece(Board& board, Draught* from, int fRow, int fCol, Draught* to, int tRow, int tCol) {
	std::string coords;	// this holds the coordinates for re-capture
	bool recap = false;	// this is a flag which goes to true when a re-capture is possible
	bool invalid = false;
	int tempRow, tempCol;	// these will be used in case of invalid re-capture coordinates
	do {	// loop through the code in case of multiple captures
		try {
			if (recap) {
				std::cin >> coords;
				if (coords == "0") break;	// if the user does not desire to capture another piece, then exit
				else if (coords.size() != 2)
					throw InvalidMoveException();
				BoardCell destCell(coords);	// convert user input coordinates into indexes for the board array
											// the current piece coordinates are now the TO coordinates from the previous capture
				tempRow = fRow;
				tempCol = fCol;
				fRow = tRow;
				fCol = tCol;
				from = to;						// set FROM pointer to point to TO since the position was changed
				tRow = destCell.getRow();		// set the destination row to the one specified by the user
				tCol = destCell.getColumn();	// set the destination column to the one specified by the user
			}
			// since kings can move forward and backwards, colour does not matter to them
			// for white pieces, men move to increasing row values (thus TO row is more than FROM row)
			if (from->isWhite() && !from->isKing() && tRow == fRow + 2) {
				if (tCol == fCol + 2 && board.getPieceAt(fRow + 1, fCol + 1) != nullptr && !board.getPieceAt(fRow + 1, fCol + 1)->isWhite()) {	// check whether there is a piece that can be jumped over and deleted
					delete board.getPieceAt(fRow + 1, fCol + 1);	// delete piece from the board
					board.setPieceAt(fRow + 1, fCol + 1);	// and then set it to point to null for display purposes
				}

				else if (tCol == fCol - 2 && board.getPieceAt(fRow + 1, fCol - 1) != nullptr && !board.getPieceAt(fRow + 1, fCol - 1)->isWhite()) {
					delete board.getPieceAt(fRow + 1, fCol - 1);
					board.setPieceAt(fRow + 1, fCol - 1);
				}
				else
					throw InvalidMoveException();
			}

			// for red pieces, men move to decreasing row values (thus TO row is less than FROM row)
			else if (!from->isWhite() && !from->isKing() && tRow == fRow - 2) {
				if (tCol == fCol + 2 && board.getPieceAt(fRow - 1, fCol + 1) != nullptr && board.getPieceAt(fRow - 1, fCol + 1)->isWhite()) {		// check whether there is a piece that can be jumped over and deleted
					delete board.getPieceAt(fRow - 1, fCol + 1);
					board.setPieceAt(fRow - 1, fCol + 1);
				}

				else if (tCol == fCol - 2 && board.getPieceAt(fRow - 1, fCol - 1) != nullptr && board.getPieceAt(fRow - 1, fCol - 1)->isWhite()) {
					delete board.getPieceAt(fRow - 1, fCol - 1);
					board.setPieceAt(fRow - 1, fCol - 1);
				}
				else
					throw InvalidMoveException();
			}
			// kings can move in four different ways (up and right; up and left; down and right; down and left)
			else if (from->isKing()) {
				if (tCol == fCol + 2 && tRow == fRow + 2 && board.getPieceAt(fRow + 1, fCol + 1) != nullptr && board.getPieceAt(fRow + 1, fCol + 1)->isWhite() != from->isWhite()) {	// check whether there is a piece that can be jumped over and deleted
					delete board.getPieceAt(fRow + 1, fCol + 1);	// delete piece from the board
					board.setPieceAt(fRow + 1, fCol + 1);	// and then set it to point to null for display purposes
				}

				else if (tRow == fRow + 2 && tCol == fCol - 2 && board.getPieceAt(fRow + 1, fCol - 1) != nullptr && board.getPieceAt(fRow + 1, fCol - 1)->isWhite() != from->isWhite()) {
					delete board.getPieceAt(fRow + 1, fCol - 1);
					board.setPieceAt(fRow + 1, fCol - 1);
				}
				else if (tRow == fRow - 2 && tCol == fCol + 2 && board.getPieceAt(fRow - 1, fCol + 1) != nullptr && board.getPieceAt(fRow - 1, fCol + 1)->isWhite() != from->isWhite()) {
					delete board.getPieceAt(fRow - 1, fCol + 1);
					board.setPieceAt(fRow - 1, fCol + 1);
				}

				else if (tCol == fCol - 2 && tRow == fRow - 2 && board.getPieceAt(fRow - 1, fCol - 1) != nullptr && board.getPieceAt(fRow - 1, fCol - 1)->isWhite() != from->isWhite()) {
					delete board.getPieceAt(fRow - 1, fCol - 1);
					board.setPieceAt(fRow - 1, fCol - 1);
				}
				else
					throw InvalidMoveException();
			}
			else
				throw InvalidTypeException(from->isKing());	// error corresponding to the type of piece is thrown
			board.setPieceAt(tRow, tCol, from);
			to = from;
			board.setPieceAt(fRow, fCol);
			from = nullptr;
			// give the user the option to capture another piece if it is possible
			if (possCapture(board, to, tRow, tCol)) {	// check whether there are any possible captures
				board.displayBoard();	// display the board before recapturing (if the player wishes)
				std::cout << "You can capture another opponent piece with the piece you just moved. Enter the board coordinate to capture once again or press 0 to stay." << std::endl;
				recap = true;	// set the possible recapture flag to true
			}
			else
				break;
		}
		catch (InvalidMoveException& e) {
			std::cerr << e.getMessage() << std::endl;	// display error message corresponding to an invalid move command
			invalid = true;
		}
		catch (InvalidTypeException& e) {
			std::cerr << e.getMessage() << std::endl;	// display error message corresponding to an invalid piece type move
			invalid = true;
		}
		catch (OutOfBoardException& e) {
			std::cerr << e.getMessage() << std::endl;	// display error message corresponding to tryting to access an out of board cell
			invalid = true;
		}
		if (invalid && recap) {
			tRow = fRow;
			tCol = fCol;
			fRow = tempRow;
			fCol = tempCol;
			invalid = false;
		}
		else if (invalid && !recap)	// if an exception was thrown while not in a recapturing state, return false
			return false;
	} while (recap);	// if the user does not input 0, keep capturing
	if (to->canKing(tRow)) {	// if a man piece reaches the opposite end of the board it gets promoted to a king with the same colour
		bool white = to->isWhite();
		delete to;	// de-allocate memory for man draught
		Draught* pking = new King(white);	
		// replace the deleted man draught with a king
		board.setPieceAt(tRow, tCol, pking);
		to = board.getPieceAt(tRow, tCol);	// set the to pointer to point at the King draught now
	}
	return true;	// return successful capture
}

bool GameController::possCapture(Board& board, Draught* from, int x, int y) const {
	bool possible = false;
	// the following if statements check whether the piece which is to be jumped is the opponent's one and is valid
	// it also checks whether the squares are in the actual board and do not exceed the size since out of range squares are returned to be nullptrs
	// in the case of a king
	if (from->isKing() && ((board.getPieceAt(x + 2, y + 2) == nullptr && x + 2 < SIZE && y + 2 < SIZE && board.getPieceAt(x + 1, y + 1) != nullptr && from->isWhite() != board.getPieceAt(x + 1, y + 1)->isWhite()) ||
		(board.getPieceAt(x + 2, y - 2) == nullptr && x + 2 < SIZE && y - 2 >= 0 && board.getPieceAt(x + 1, y - 1) != nullptr && from->isWhite() != board.getPieceAt(x + 1, y - 1)->isWhite()) ||
		(board.getPieceAt(x - 2, y + 2) == nullptr && x - 2 >= 0 && y + 2 < SIZE && board.getPieceAt(x - 1, y + 1) != nullptr && from->isWhite() != board.getPieceAt(x - 1, y + 1)->isWhite()) ||
		(board.getPieceAt(x - 2, y - 2) == nullptr && x - 2 >= 0 && y - 2 >= 0 && board.getPieceAt(x - 1, y - 1) != nullptr && from->isWhite() != board.getPieceAt(x - 1, y - 1)->isWhite()))) {
		possible = true;
	}
	// when a man draught is white
	else if ((!from->isKing() && from->isWhite()) && ((board.getPieceAt(x + 2, y + 2) == nullptr && x + 2 < SIZE && y + 2 < SIZE && board.getPieceAt(x + 1, y + 1) != nullptr && !board.getPieceAt(x + 1, y + 1)->isWhite()) ||
		(board.getPieceAt(x + 2, y - 2) == nullptr && x + 2 < SIZE && y - 2 >= 0 && board.getPieceAt(x + 1, y - 1) != nullptr && !board.getPieceAt(x + 1, y - 1)->isWhite()))) {
		possible = true;
	}
	// when a man draught is red
	else if ((!from->isKing() && !from->isWhite()) && ((board.getPieceAt(x - 2, y + 2) == nullptr && x - 2 < SIZE && y + 2 && board.getPieceAt(x - 1, y + 1) != nullptr && board.getPieceAt(x - 1, y + 1)->isWhite()) ||
		(board.getPieceAt(x - 2, y - 2) == nullptr && x - 2 >= 0 && y - 2 >= 0 && board.getPieceAt(x - 1, y - 1) != nullptr && board.getPieceAt(x - 1, y - 1)->isWhite()))) {
		possible = true;
	}

	return possible;	// return whether or not a capture is possible
}

bool GameController::checkIfWinner(Board& board) {
	// these keep track of amount of white/red pieces on the board
	unsigned int white_counter = 0;
	unsigned int red_counter = 0;
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (board.getPieceAt(i, j) == nullptr) continue;
			else if ((board.getPieceAt(i, j))->isWhite())
				white_counter++;		// if there is a white piece anywhere on the board, then white did not lose yet...
			else if (!(board.getPieceAt(i, j))->isWhite())
				red_counter++;			// ...same for red pieces
		}
	}
	if (white_counter == 0) {
		board.displayBoard();
		std::cout << "Player 1 <Red> is the winner with " << red_counter << " pieces left on the board!" << std::endl;	// declare red as the winner if there are no white pieces on the board
	}
	else if (red_counter == 0) {
		board.displayBoard();
		std::cout << "Player 2 <White> is the winner with " << white_counter << " pieces left on the board!" << std::endl; // declare white as the winner if there are no red pieces on the board
	}
	else {
		board.getTurn() == 1 ? board.setTurn(2) : board.setTurn(1);	// give the turn to the other player
		board.displayBoard();	// display the board
		return false;
	}
	return true;
}