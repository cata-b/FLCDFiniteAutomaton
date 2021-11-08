#pragma once
#include "FA.hpp"
#include <string>
#include <fstream>

class CharFAReader
{
public:
	static FA<char, char> readFA(std::string filename)
	{
		using namespace std;

		unordered_set<char> initial_states;
		unordered_set<char> final_states;
		vector<tuple<char, char, char>> transitions;

		ifstream in(filename);
		string line;
		size_t step = 0;
		while (getline(in, line))
		{
			if (line.size() == 0)
				continue;
			switch (step)
			{
			case 0: // reading initial states
				if (line.size() > 1)
					++step;
				else
				{
					initial_states.insert(line[0]);
					break;
				}
			case 1: // reading transitions
				if (line.size() < 3)
					step += 1;
				else
				{
					transitions.push_back(make_tuple(line[0], line[1], line[2]));
					break;
				}
			case 2: // reading final states
				final_states.insert(line[0]);
				break;
			}
		}

		return FA<char, char>{ transitions, initial_states, final_states };
	}
};

