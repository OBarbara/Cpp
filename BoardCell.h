#include <string>
#include "OutOfBoardException.h"

class BoardCell
{
private:
	std::string m_coords;	// store the coordinates entered by the user
public:
	BoardCell(std::string coordinates)
		: m_coords(coordinates) {}
	int getRow() const;		// returns the row corresponding to the square
	int getColumn() const;	// returns the column corresponding to the square
};