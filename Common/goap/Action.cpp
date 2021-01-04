#include "Action.h"
#include "WorldState.h"

#include <cassert>
using namespace PEPEngine;

goap::Action::Action() : cost_(0)
{
	 target = std::make_shared<Common::Transform>();
	 target->SetPosition(Vector3(0, 0, 0));
}

goap::Action::Action(std::string name, int cost) : Action()
{
	// Because delegating constructors cannot initialize & delegate at the same time...
	name_ = name;
	cost_ = cost;
	target->SetPosition(Vector3(0, 0, 0));
}

bool goap::Action::perform(WorldState& ws)
{
	ws = actOn(ws);
	isDone = true;
	return isDone;
}

bool goap::Action::IsRequiresInRange() const
{
	return requiresInRange;
}


bool goap::Action::getInRange() const
{
	return inRange;
}

bool goap::Action::getIsDone() const
{
	return isDone;
}

void goap::Action::setInRange(bool val)
{
	inRange = val;
}

void goap::Action::setRequiresInRange(bool value)
{
	requiresInRange = value;
}

void goap::Action::setCost(int cost)
{
	cost_ = cost;
}


void goap::Action::postPerform()
{
	setInRange(false);
	isDone = false;
}

Vector3 goap::Action::getSteering()
{
	return Vector3(0, 0, 0);
}


bool goap::Action::operableOn(const WorldState& ws) const
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

goap::WorldState goap::Action::actOn(const WorldState& ws) const
{
	WorldState tmp(ws);
	for (const auto& effect : effects_)
	{
		tmp.setVariable(effect.first, effect.second);
	}
	return tmp;
}
