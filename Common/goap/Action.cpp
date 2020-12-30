#include "Action.h"
#include "WorldState.h"

#include <cassert>
using namespace PEPEngine;
goap::Action::Action() : cost_(0) {
	target = std::make_shared<Common::Transform>();
}

goap::Action::Action(std::string name, int cost) : Action() {
    // Because delegating constructors cannot initialize & delegate at the same time...
    name_ = name;
    cost_ = cost;
}

bool goap::Action::perform(WorldState& ws)
{
    ws= actOn(ws);
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


bool goap::Action::operableOn(const WorldState& ws) const {
    for (const auto& precond : preconditions_) {
        try {
            if (ws.vars_.at(precond.first) != precond.second) {
                return false;
            }
        } catch (const std::out_of_range&) {
            return false;
        }
    }
    return true;
}

goap::WorldState goap::Action::actOn(const WorldState& ws) const {
    goap::WorldState tmp(ws);
    for (const auto& effect : effects_) {
        tmp.setVariable(effect.first, effect.second);
    }
    return tmp;
}

