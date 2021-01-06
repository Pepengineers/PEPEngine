#include "PickupAction.h"

#include "AIComponent.h"

PickupAction::PickupAction() : Action()
{
	setEffect(HAS_WEAPON, true);
	setPrecondition(HAS_WEAPON, false);
	
	name_ = std::string("pickup");
	cost_ = 5;
	setRequiresInRange(true);
	target->SetPosition(Vector3(100, 100, 0));
}

void PickupAction::prePerform(PEPEngine::Common::GameObject* g)
{
}

void PickupAction::postPerform(PEPEngine::Common::GameObject* g)
{
	auto ai = g->GetComponent<AIComponent>().get()->gameObject->GetTransform()->GetScale();

	g->GetComponent<AIComponent>().get()->gameObject->GetTransform()->SetScale(ai + Vector3(1.2, 1.2, 1.2));
}
