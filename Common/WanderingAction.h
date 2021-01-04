#pragma once
#include "goap/Action.h"

class WanderingAction :
    public PEPEngine::goap::Action
{
public:
	WanderingAction();
	WanderingAction(std::string name, int cost);
	 void prePerform() override ;
	
};

