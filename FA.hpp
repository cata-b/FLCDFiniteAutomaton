#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

template <typename state_type, typename transition_type>
class FA
{
private:
	bool _is_deterministic = false;

	std::vector<state_type> _states;
	std::unordered_set<transition_type> _alphabet;

	std::unordered_set<size_t> _initial_state_ids;
	std::unordered_set<size_t> _final_state_ids;

	// for each state -> map of transitions to other states (set of state indices, 
	// because a transition may lead to more than one state if the FA is not deterministic)
	std::vector<std::unordered_map<transition_type, std::unordered_set<size_t>>> _transitions;

public:
	FA(const std::vector<std::tuple<state_type, transition_type, state_type>>& transitions, 
		const std::unordered_set<state_type>& initial_states, 
		const std::unordered_set<state_type>& final_states);

	bool is_deterministic() const { return _is_deterministic; };

	template <std::input_iterator iter_type>
	bool accepts(iter_type begin, iter_type end) const;

	const std::vector<state_type>& states() const { return _states; };
	std::vector<state_type> initial_states() const;
	std::vector<state_type> final_states() const;
	const std::unordered_set<transition_type>& alphabet() const { return _alphabet; };
	std::vector<std::pair<state_type, std::vector<std::pair<transition_type, std::vector<state_type>>>>> transitions() const;
};

template<typename state_type, typename transition_type>
inline FA<state_type, transition_type>::FA(const std::vector<std::tuple<state_type, transition_type, state_type>>& transitions, const std::unordered_set<state_type>& initial_states, const std::unordered_set<state_type>& final_states)
{
	std::unordered_map<state_type, size_t> state_uniq; // state -> id in state vector
	state_uniq.reserve(initial_states.size() + final_states.size());

	_states.reserve(initial_states.size() + final_states.size());
	
	_is_deterministic = true;

	if (initial_states.size() != 1)
		_is_deterministic = false;

	for (const auto& state : initial_states) 
	{
		_initial_state_ids.insert(_states.size());
		state_uniq[state] = _states.size();
		_states.push_back(state);
	}
	
	for (const auto& state : final_states)
	{
		auto existing_state = state_uniq.find(state);
		if (existing_state != state_uniq.end())
			_final_state_ids.insert(existing_state->second);
		else
		{
			_final_state_ids.insert(_states.size());
			state_uniq[state] = _states.size();
			_states.push_back(state);
		}
	}
		
	for (const auto& transition : transitions)
	{
		const auto& [init, trans, fin] = transition;

		_alphabet.insert(trans);

		size_t init_id;
		auto existing_init = state_uniq.find(init);
		if (existing_init != state_uniq.end())
			init_id = existing_init->second;
		else
		{
			init_id = state_uniq[init] = _states.size();
			_states.push_back(init);
		}

		size_t fin_id;
		auto existing_fin = state_uniq.find(fin);
		if (existing_fin != state_uniq.end())
			fin_id = existing_fin->second;
		else
		{
			fin_id = state_uniq[fin] = _states.size();
			_states.push_back(fin);
		}

		while (init_id > _transitions.size())
			_transitions.push_back(std::unordered_map<transition_type, std::unordered_set<size_t>>{ });
		
		if (init_id == _transitions.size())
			_transitions.push_back(std::unordered_map<transition_type, std::unordered_set<size_t>>{ { trans, std::unordered_set<size_t>{ fin_id } } });	
		else
		{
			if (_transitions[init_id].find(trans) != _transitions[init_id].end())
			{
				_transitions[init_id][trans].insert(fin_id);
				if (_transitions[init_id][trans].size() > 1)
					_is_deterministic = false;
			}
			else
				_transitions[init_id][trans] = std::unordered_set<size_t>{ fin_id };
		}
	}

}

template<typename state_type, typename transition_type>
inline std::vector<state_type> FA<state_type, transition_type>::initial_states() const
{
	std::vector<state_type> result;
	result.reserve(_initial_state_ids.size());
	for (auto id : _initial_state_ids)
		result.push_back(_states[id]);
	return result;
}

template<typename state_type, typename transition_type>
inline std::vector<state_type> FA<state_type, transition_type>::final_states() const
{
	std::vector<state_type> result;
	result.reserve(_final_state_ids.size());
	for (auto id : _final_state_ids)
		result.push_back(_states[id]);
	return result;
}

template<typename state_type, typename transition_type>
inline std::vector<std::pair<state_type, std::vector<std::pair<transition_type, std::vector<state_type>>>>> FA<state_type, transition_type>::transitions() const
{
	std::vector<std::pair<state_type, std::vector<std::pair<transition_type, std::vector<state_type>>>>> result;
	result.reserve(_transitions.size());
	for (size_t i = 0; i < _states.size(); ++i)
	{
		const auto& current_state = _states[i];
		const std::unordered_map<transition_type, std::unordered_set<size_t>>& transitions = _transitions[i];
		std::vector<std::pair<transition_type, std::vector<state_type>>> transitions_out;
		transitions_out.reserve(transitions.size());
		for (const auto& entry : transitions)
		{
			const auto& [transition, end_states] = entry;
			std::vector<state_type> end_states_out;
			end_states_out.reserve(end_states.size());
			for (const auto& end_state : end_states)
				end_states_out.push_back(_states[end_state]);
			transitions_out.push_back({ transition, std::move(end_states_out) });
		}
		result.push_back({ current_state, std::move(transitions_out) });
	}
	return result;
}

template<typename state_type, typename transition_type>
template<std::input_iterator iter_type>
inline bool FA<state_type, transition_type>::accepts(iter_type begin, iter_type end) const
{
	if (!_is_deterministic)
		throw std::runtime_error("This finite automaton is not deterministic!");

	auto current_state = *_initial_state_ids.begin();
	auto current_item = begin;
	while (current_item != end)
	{
		const std::unordered_map<transition_type, std::unordered_set<size_t>>& transitions = _transitions[current_state];
		auto transition_it = transitions.find(*current_item);
		if (transition_it == transitions.end())
			return false;
		auto& next_state_set = transition_it->second;
		if (next_state_set.size() == 0)
			return false;
		current_state = *next_state_set.begin();
		++current_item;
	}
	return _final_state_ids.find(current_state) != _final_state_ids.end();
}