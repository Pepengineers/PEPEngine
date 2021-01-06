#include "AttackAction.h"

#include "AIComponent.h"

void AttackAction::prePerform(PEPEngine::Common::GameObject* g)
{
	auto player = g->GetComponent<AIComponent>();
	auto playerTransform = player->getPlayer()->GetTransform();
	target = playerTransform;
}

void AttackAction::postPerform(PEPEngine::Common::GameObject* g)
{
	
}
