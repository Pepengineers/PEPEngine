#include "WanderingAction.h"

#include "AIComponent.h"

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

void WanderingAction::prePerform(PEPEngine::Common::GameObject* g)
{

	auto x = g->GetTransform()->GetWorldPosition().x;
	auto z = g->GetTransform()->GetWorldPosition().x;
  target->SetPosition(Vector3( x + rand() % 200 - 100, 1, z + rand() % 200 - 100));
}

void WanderingAction::postPerform(PEPEngine::Common::GameObject* g)
{
	

	

}




