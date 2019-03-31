#pragma once
#include "BoardException.h"
class InvalidPieceException :
	public BoardException
{
public:
	virtual std::string getMessage() const {
		return "Unable to move piece. Make sure that you are attempting to move one of your pieces and that the destination square is vacant.";
	}
};

