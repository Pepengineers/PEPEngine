#include "WanderingAction.h"

WanderingAction::WanderingAction() : Action()
{
	setEffect(WANDERING, false);
	name_ = std::string("wandering");
	cost_ = 1;
	setRequiresInRange(true);
	target->SetPosition(Vector3(100, 100, 0));
}

WanderingAction::WanderingAction(std::string name, int cost) : Action(name, cost)
{
	setEffect(WANDERING, false);
	setRequiresInRange(true);
	target->SetPosition(Vector3(100, 100, 0));
}

void WanderingAction::prePerform()
{
	target->SetPosition(Vector3(rand() % 200, 1, rand() % 200));
}
