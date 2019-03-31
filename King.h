#pragma once
#include "Draught.h"
class King :
	public Draught
{
public:
	King(bool white)
		: Draught(white, true) {
		// set the symbol for the particular piece
		if (white)
			m_symbol = '#';
		else
			m_symbol = '*';
	}

	virtual bool canKing(int row) const{ return false; }	// kings cannot become kings again
};

