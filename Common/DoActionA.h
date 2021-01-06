#pragma once
#include "Action.h"

class DoActionA :
	public Action
{
public:
	DoActionA() : Action()
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(1, 1, 1));
		setCost(1);
		setPrecondition(POKE_B, true);
	}

	DoActionA(std::string name, int cost) : Action(name, cost)
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(1, 1, 1));
		setCost(cost);
		setEffect(POKE_A, true);
		
	}
	void prePerform(PEPEngine::Common::GameObject*) override;
	void postPerform(PEPEngine::Common::GameObject*) override;
};
