#pragma once
#include "goap/Action.h"

class DoActionB :
	public PEPEngine::goap::Action
{
	DoActionB() : Action()
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(10, 10, 1));
		setEffect(POKE_A, false);
		setEffect(POKE_B, true);
	}

	DoActionB(std::string name, int cost) : Action(name, cost)
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(10, 10, 1));
		setEffect(POKE_A, false);
		setEffect(POKE_B, true);
	}
};
