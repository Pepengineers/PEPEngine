#pragma once
#include "Action.h"
class GoToAction :
    public Action
{
	GoToAction() : Action()
	{
		name_ = std::string("GoTo");
		setCost(1);
		
	}
	void prePerform(PEPEngine::Common::GameObject*) override;
	void postPerform(PEPEngine::Common::GameObject*) override;
};

