#pragma once
#include "Action.h"

class AttackAction :
    public Action
{
public:
	AttackAction() : Action()
	{
		name_ = std::string("AttackAction");
		setCost(5);
		setRequiresInRange(true);
		setPrecondition(NEED_ATTACK, true);
		setPrecondition(HAS_WEAPON, true);
		setEffect(TARGET_ALIVE, false);
		
	}
	void prePerform(PEPEngine::Common::GameObject*) override;
	void postPerform(PEPEngine::Common::GameObject*) override;
};

