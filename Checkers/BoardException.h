#pragma once
#include <string>
class BoardException
{
public:
	virtual std::string getMessage() const {
		return "An unknown error has occured.";
	}
};

