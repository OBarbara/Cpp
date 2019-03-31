#pragma once
#include "BoardException.h"
class InvalidTypeException :
	public BoardException
{
private:
	bool m_king;
public:
	InvalidTypeException(bool is_king)
		: m_king(is_king) {}

	virtual std::string getMessage() const {	// display a different error for different pieces
		if (!m_king)
			return "This man draught cannot move in that way. Red men can only move diagonally and upwards, while white men diagonally downwards.";
		else
			return "Kings cannot move in that way. They can move diagonally, both forwards and backwards by one square.";
	}
};

