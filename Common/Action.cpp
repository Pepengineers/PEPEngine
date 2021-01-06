#include "pch.h"
#include "Action.h"
#include "AIComponent.h"


	Action::Action() : cost_(0)
	{
		target = std::make_shared<PEPEngine::Common::Transform>();
		target->SetPosition(Vector3(0, 0, 0));
	}

	Action::Action(std::string name, int cost) : Action()
	{
		// Because delegating constructors cannot initialize & delegate at the same time...
		name_ = name;
		cost_ = cost;
		target->SetPosition(Vector3(0, 0, 0));
	}

	bool Action::perform(WorldState& ws)
	{
		ws = actOn(ws);
		isDone = true;
		return isDone;
	}

	bool Action::IsRequiresInRange() const
	{
		return requiresInRange;
	}


	bool Action::getInRange() const
	{
		return inRange;
	}

	bool Action::getIsDone() const
	{
		return isDone;
	}

	void Action::setInRange(bool val)
	{
		inRange = val;
	}

	void Action::setRequiresInRange(bool value)
	{
		requiresInRange = value;
	}

	void Action::setCost(int cost)
	{
		cost_ = cost;
	}

	void Action::prePerform(PEPEngine::Common::GameObject* g)
	{

	}


	void Action::postPerform(PEPEngine::Common::GameObject* g)
	{


		setInRange(false);
		isDone = false;
	}

	Vector3 Action::getSteering()
	{
		return Vector3(0, 0, 0);
	}


	bool Action::operableOn(const WorldState& ws) const
	{
		for (const auto& precond : preconditions_)
		{
			try
			{
				if (ws.vars_.at(precond.first) != precond.second)
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

	WorldState Action::actOn(const WorldState& ws) const
	{
		WorldState tmp(ws);
		for (const auto& effect : effects_)
		{
			tmp.setVariable(effect.first, effect.second);
		}
		return tmp;
	}