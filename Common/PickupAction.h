#pragma once

#include "Action.h"

class PickupAction :
    public Action
{
public:
	PickupAction();
	void prePerform(PEPEngine::Common::GameObject*) override;
	void postPerform(PEPEngine::Common::GameObject*) override;
};

