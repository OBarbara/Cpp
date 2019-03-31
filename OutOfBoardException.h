#pragma once
#include "BoardException.h"
class OutOfBoardException :
	public BoardException
{
public:
	virtual std::string getMessage() const {
		return "Error. Attempting to access a board square which is non-existent.";
	}
};

