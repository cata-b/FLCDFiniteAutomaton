#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <optional>
#include "FA.hpp"
using namespace std;

FA<char, char> read_from_file(string filename);

template <std::input_iterator iter_type>
string make_string(iter_type begin, iter_type end);

void display_fa_data(const FA<char, char>& fa);

int main() 
{	
	optional<FA<char, char>> fa;

	string menu_text =
		"0. Exit\n"
		"1. Read a finite automaton from a file\n"
		"2. Display elements of the current automaton\n"
		"3. If the current automaton is deterministic, check if it accepts a string\n";
	vector<function<void()>> menu_functions{
		[&fa]() { // read from file
			cout << "Filename: ";
			string fn;
			cin >> fn;
			fa = read_from_file(fn);
		},
		[&fa]() {
			if (!fa.has_value()) 
			{
				cout << "No automaton has been read from file.\n";
				return;
			}
			display_fa_data(fa.value());
		},
		[&fa]() {
			if (!fa.has_value())
			{
				cout << "No automaton has been read from file.\n";
				return;
			}
			if (!fa.value().is_deterministic())
			{
				cout << "The automaton is not deterministic.\n";
				return;
			}
			string test;
			cout << "Enter sequence: ";
			cin >> test;
			cout << "The automaton " << (fa.value().accepts(test.begin(), test.end()) ? "accepts " : "does not accept ") << "this sequence.\n";
		}
	};

	int choice;
	do
	{
		cout << menu_text;
		cin >> choice;
		if (choice > 0 && choice <= menu_functions.size())
			menu_functions[choice - 1]();
	} while (choice != 0);

	return 0;
}

FA<char, char> read_from_file(string filename)
{
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

void display_fa_data(const FA<char, char>& fa)
{
	cout <<
		"Choose what to display: \n"
		"0. Initial states\n"
		"1. Final states\n"
		"2. All states\n"
		"3. Alphabet\n"
		"4. Transitions\n"
		"5. Whether or not the automaton is deterministic\n";

	vector<function<void()>> menu_options{
		[&fa]() {
			auto is = fa.initial_states(); 
			cout << "Initial states: " << make_string(is.begin(), is.end()) << '\n';
		},
		[&fa]() {
			auto fs = fa.final_states();
			cout << "Final states: " << make_string(fs.begin(), fs.end()) << '\n';
		},
		[&fa]() {
			auto s = fa.states();
			cout << "States: " << make_string(s.begin(), s.end()) << '\n';
		},
		[&fa]() {
			auto a = fa.alphabet();
			cout << "Alphabet: " << make_string(a.begin(), a.end()) << '\n';
		},
		[&fa]() {
			auto t = fa.transitions();
			cout << "Transitions: \n";
			for (auto& entry : t)
			{
				auto& [curr_st, transs] = entry;
				cout << curr_st << '\n';
				for (auto& trans_with_finals : transs)
				{
					auto& [trans, finals] = trans_with_finals;
					cout << ' ' << trans << '\n';
					for (auto& fin : finals)
						cout << "  " << fin << '\n';
				}
			}
		},
		[&fa]() {
			cout << "The automaton is " << (fa.is_deterministic() ? "" : "not") << " deterministic.\n";
		}
	};
	int choice;
	cin >> choice;
	if (choice >= 0 && choice < menu_options.size())
		menu_options[choice]();
}

template<std::input_iterator iter_type>
string make_string(iter_type begin, iter_type end)
{
	ostringstream result("");
	while (begin != end)
		result << *(begin++);
	return result.str();
}
