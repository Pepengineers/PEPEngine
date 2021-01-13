#include "FleeAction.h"

#include "AIComponent.h"

void FleeAction::prePerform(PEPEngine::Common::GameObject* g)
{
	auto ai = g->GetComponent<AIComponent>();
	auto selfTransform = ai->gameObject->GetTransform()->GetWorldPosition();
	auto playerTransform = ai->getPlayer()->GetTransform()->GetWorldPosition();
	auto vectorFromPlayer = (selfTransform - playerTransform);
	
	auto project = Vector3(vectorFromPlayer.x, 0, vectorFromPlayer.z);
	project.Normalize();
	target->SetPosition( selfTransform + project * fleeRange/5);
	
}

void FleeAction::postPerform(PEPEngine::Common::GameObject* g)
{
	auto ai = g->GetComponent<AIComponent>();
	auto selfTransform = ai->gameObject->GetTransform()->GetWorldPosition();
	auto playerTransform = ai->getPlayer()->GetTransform()->GetWorldPosition();
	auto vectorFromPlayer = (selfTransform - playerTransform);
	
	if (vectorFromPlayer.Length() < fleeRange)
	{
		ai->setWorldState(NEED_FLEE, true);
	}
	
}
