#pragma once
#include "BoardException.h"
class InvalidMoveException :
	public BoardException
{
public:
	virtual std::string getMessage() const {
		return "The move coordinates entered are invalid.";
	}
};

