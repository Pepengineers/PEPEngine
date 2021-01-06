#pragma once

#include "Action.h"

class FleeAction :
    public Action
{
public:
	FleeAction() : Action()
	{
		name_ = std::string("FleeAction");
		setCost(0);
		setPrecondition(NEED_FLEE, true);
		setEffect(NEED_FLEE, false);
		setRequiresInRange(true);
		
	}
	void prePerform(PEPEngine::Common::GameObject*) override;
	void postPerform(PEPEngine::Common::GameObject*) override;
	float fleeRange = 200;
};

