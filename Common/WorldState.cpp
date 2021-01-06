#include "pch.h"
#include "WorldState.h"



	WorldState::WorldState(const std::string name) : priority_(0), name_(name)
	{
		//nop
	}

	void WorldState::setVariable(const int var_id, const bool value)
	{
		vars_[var_id] = value;
	}

	bool WorldState::getVariable(const int var_id) const
	{
		return vars_.at(var_id);
	}


	bool WorldState::operator==(const WorldState& other) const
	{
		return (vars_ == other.vars_);
	}

	bool WorldState::meetsGoal(const WorldState& goal_state) const
	{
		for (const auto& kv : goal_state.vars_)
		{
			try
			{
				if (vars_.at(kv.first) != kv.second)
				{
					return false;
				}
			}
			catch (const std::out_of_range&)
			{
				return false;
			}
		}
		return true;
	}

	int WorldState::distanceTo(const WorldState& goal_state) const
	{
		int result = 0;

		for (const auto& kv : goal_state.vars_)
		{
			auto itr = vars_.find(kv.first);
			if (itr == end(vars_) || itr->second != kv.second)
			{
				++result;
			}
		}

		return result;
	}
