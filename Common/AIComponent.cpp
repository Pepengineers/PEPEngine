#include "AIComponent.h"

#include "DoActionA.h"
#include "DoActionB.h"
#include "WanderingAction.h"
#include "GameObject.h"
#include "Transform.h"

enum test { TARGET_ACQUIRED, TARGET_DEAD, IN_LOCATION };

AIComponent::AIComponent()
{
	this->SetWorldState();
	this->SetActionList();
	this->currentState_ = FSMState::Idle;
}

void AIComponent::SetActionList()
{
	availableActions.push_back(new DoActionA);
	availableActions.push_back(new DoActionB);
	availableActions.push_back(new WanderingAction);
}


void AIComponent::SetWorldState()
{
	worldState.setVariable(PEPEngine::goap::Action::POKE_A, false);
	worldState.setVariable(PEPEngine::goap::Action::POKE_B, false);
	worldState.setVariable(PEPEngine::goap::Action::WANDERING, false);
	goal.setVariable(PEPEngine::goap::Action::POKE_A, true);
	goal.setVariable(PEPEngine::goap::Action::POKE_B, true);
	goal.setVariable(PEPEngine::goap::Action::WANDERING, false);

	//goal.setVariable(PEPEngine::goap::Action::POKE_C, true);
}

void AIComponent::Update()
{
	/*if (worldState.getVariable(PEPEngine::goap::Action::POKE_A) && worldState.
		getVariable(PEPEngine::goap::Action::POKE_B))
	{
		worldState.setVariable(PEPEngine::goap::Action::POKE_A, false);
		worldState.setVariable(PEPEngine::goap::Action::POKE_B, false);

	}*/

	switch (currentState_)
	{
	case FSMState::Idle:
		{
			auto plan = planner.plan(worldState, goal, availableActions);

			if (plan.empty())
			{
				currentState_ = FSMState::Idle;
				worldState.setVariable(PEPEngine::goap::Action::WANDERING, true);
			}
			else
			{
				currentActions = plan;
				currentState_ = FSMState::PerformAction;
			}
			break;
		}
	case FSMState::MoveTo:
		{
			//TODO: Переписать под нормальное передвижение

			auto action = currentActions.back();
			auto transform = gameObject->GetTransform();
			auto current = gameObject->GetTransform()->GetWorldPosition();
			auto target = action->target.get()->GetWorldPosition();

			if (dumpTarget != target) dt = 0;
			dt += 0.0001;
			if (dt > 1) dt = 0;
			dumpTarget = target;

			auto len = (target - current);
			auto distanceTo = len.Length();
			auto lookAt = Matrix::CreateLookAt(current, target, gameObject->GetTransform()->GetUpVector()).Transpose();
			auto q = Quaternion::Slerp(gameObject->GetTransform()->GetQuaternionRotate(),
			                           Quaternion::CreateFromRotationMatrix(lookAt), dt);

			gameObject->GetTransform()->SetQuaternionRotate(q);

			if (distanceTo > 0 && distanceTo < action->range)
			{
				action->setInRange(true);
				currentState_ = FSMState::PerformAction;
			}
			else
			{
				auto dir = target - current;
				dir.Normalize();
				transform->AdjustPosition(gameObject->GetTransform()->GetForwardVector() * 0.1);
			}

			break;
		}
	case FSMState::PerformAction:
		{
			if (currentActions.empty())
			{
				currentState_ = FSMState::Idle;
			}
			else
			{
				auto action = currentActions.back();
				action->prePerform();

				if (action->IsRequiresInRange() && action->getInRange() || !action->IsRequiresInRange())
				{
					bool result = action->perform(worldState);
					if (result)
					{
						action->postPerform();
						currentActions.pop_back();
					}
					else
					{
						if (currentActions.empty())
							currentState_ = FSMState::Idle;
					}
				}
				else
				{
					currentState_ = FSMState::MoveTo;
				}
			}

			break;
		}
	}
}

void AIComponent::PopulateDrawCommand(std::shared_ptr<PEPEngine::Graphics::GCommandList> cmdList)
{
}
