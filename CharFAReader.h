#pragma once
#include "FA.hpp"
#include <string>

class CharFAReader
{
public:
	static FA<char, char> readFA(std::string filename);
};

