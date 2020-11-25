#pragma once
#include "goap/Action.h"

class DoActionA :
    public PEPEngine::goap::Action
{
	DoActionA() : Action() 
	{
		
		setRequiresInRange(true);
		target->SetPosition(Vector3(1, 1, 1));
		setEffect(POKE_A, true);
		setEffect(POKE_B, false);
		
	}

	DoActionA(std::string name, int cost) : Action( name, cost)
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(1, 1, 1));
		setEffect(POKE_A, true);
		setEffect(POKE_B, false);
		
	}
	
};

