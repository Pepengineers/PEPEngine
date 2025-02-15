#pragma once
#include "Action.h"


class DoActionB :
	public Action
{
public:
	DoActionB() : Action()
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(100, 1, 1));
		setCost(5);
		setEffect(POKE_B, true);
	}

	DoActionB(std::string name, int cost) : Action()
	{
		setRequiresInRange(true);
		target->SetPosition(Vector3(100, 1, 100));
		setEffect(POKE_B, true);
		
		setCost(cost);
		
	}
	void prePerform(PEPEngine::Common::GameObject*) override;
	void postPerform(PEPEngine::Common::GameObject*) override;
};
