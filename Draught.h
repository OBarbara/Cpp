#pragma once
#include <string>
class Draught
{
private:
	const bool m_white;	// white is true, red is false
	bool m_king;	// true is king, false is man
protected:
	char m_symbol;
public:
	Draught(bool colour, bool king)
		: m_white(colour), m_king(king) {}
	virtual ~Draught() = default;	// virtual destructor makes sure that destructor of derived classes are also executed

	bool isWhite() const { return m_white; }	// returns whether the piece is white or not (red)
	char getSymbol() const { return m_symbol; }	// returns the symbol representing the piece on the board
	bool isKing() const { return m_king; }	// returns whether the piece is a king or not
	virtual bool canKing(int row) const = 0;	// returns whether the piece can be declared a king
};

