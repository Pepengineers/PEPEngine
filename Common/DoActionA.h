#pragma once
#include "goap/Action.h"

class DoActionA :
	public PEPEngine::goap::Action
{
public:
	DoActionA() : Action()
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(1, 1, 1));
		setCost(1);
		setEffect(POKE_A, true);
	}

	DoActionA(std::string name, int cost) : Action(name, cost)
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(1, 1, 1));
		setCost(cost);
		setEffect(POKE_A, true);
	}

	void prePerform() override;
};
