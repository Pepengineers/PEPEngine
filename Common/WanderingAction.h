#pragma once
#include "Action.h"

class WanderingAction :
    public Action
{
public:
	WanderingAction();
	WanderingAction(std::string name, int cost);
	
	 void prePerform(PEPEngine::Common::GameObject*) override;
	 void postPerform(PEPEngine::Common::GameObject*) override;
	
};

