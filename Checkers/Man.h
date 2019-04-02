#pragma once
#include "Draught.h"
#include <iostream>
class Man :
	public Draught
{
public:
	Man(bool white)
		: Draught(white, false) {
		if (white)
			m_symbol = 'x';
		else
			m_symbol = 'o';
	}

	virtual bool canKing(int row)const;
};

