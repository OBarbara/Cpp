#include "Man.h"

bool Man::canKing(int row) const {
	// check whether the piece is eligible for promotion by checking its colour
	if ((isWhite() && row == 7) || (!isWhite() && row == 0)) {
		std::string colour = this->isWhite() ? "white" : "red";
		std::cout << "A new " << colour << " king was promoted!" << std::endl;	// display message to the user saying that a new king was promoted
		return true;
	}
	else return false;
}