#include "pch.h"
#include "AIComponent.h"

#include "AttackAction.h"
#include "DoActionA.h"
#include "DoActionB.h"
#include "FleeAction.h"
#include "PickupAction.h"
#include "WanderingAction.h"


using namespace PEPEngine::Common;


void AIComponent::Serialize(json& j)
{
	j["Type"] = ComponentID;
}

void AIComponent::Deserialize(json& j)
{
	Initialize();
}

void AIComponent::Initialize()
{
	this->SetStartWorldState();
	this->SetActionList();
	this->currentState_ = FSMState::Idle;
	this->currentAIType = AIType::Frightened;
	rotationSpeed = 0.02;
	movementSpeed = 0.7 + (static_cast<float>(rand()) / static_cast<float>((RAND_MAX)) * 0.2);
}

AIComponent::AIComponent() : Component()
{
	Initialize();
}

void AIComponent::SetActionList()
{
	availableActions.push_back(new DoActionA);
	availableActions.push_back(new DoActionB);
	availableActions.push_back(new WanderingAction);
	availableActions.push_back(new AttackAction);
	availableActions.push_back(new PickupAction);
	availableActions.push_back(new FleeAction);
}


void AIComponent::SetStartWorldState()
{
	worldState.setVariable(Action::HAS_WEAPON, false);
	worldState.setVariable(Action::NEED_ATTACK, false);
	worldState.setVariable(Action::NEED_FLEE, false);
	worldState.setVariable(Action::NEED_WANDERING, false);
	worldState.setVariable(Action::TARGET_ALIVE, true);
}

void AIComponent::gotaGoFast()
{
	movementSpeed *= 2.5;
}

void AIComponent::setWorldState(int id, bool state)
{
	auto currentValue = worldState.getVariable(id);
	if (currentValue == state) return;

	worldState.setVariable(id, state);
	currentActions = planner.plan(worldState, goal, availableActions);
}

void AIComponent::setGoalState(int id, bool state)
{
	auto currentValue = worldState.getVariable(id);
	if (currentValue == state) return;
	goal.setVariable(id, state);
	currentActions = planner.plan(worldState, goal, availableActions);
}

float AIComponent::getDistanceToPlayer()
{
	auto selfTransform = this->gameObject->GetTransform()->GetWorldPosition();
	auto playerTransform = this->getPlayer()->GetTransform()->GetWorldPosition();
	auto vectorFromPlayer = (selfTransform - playerTransform);
	return vectorFromPlayer.Length();
}

void AIComponent::addGlobalState(
	std::vector<std::shared_ptr<GameObject>>* objects)
{
	otherObjects = objects;
}


void AIComponent::preUpdate()
{
	switch (currentAIType)
	{
	case AIType::Passive:
		{
			break;
		}
	case AIType::Frightened:
		{
			if (this->getDistanceToPlayer() < 100)
			{
				setGoalState(Action::NEED_FLEE, false);
				setWorldState(Action::NEED_FLEE, true);
			}
			break;
		}
	case AIType::Aggressive:
		{
			if (this->getDistanceToPlayer() < 200)
			{
				setWorldState(Action::NEED_ATTACK, true);
				setGoalState(Action::TARGET_ALIVE, false);
			}
			break;
		}
	}
}

void AIComponent::Update()
{
	this->preUpdate();
	switch (currentState_)
	{
	case FSMState::Idle:
		{
			auto plan = planner.plan(worldState, goal, availableActions);

			if (plan.empty())
			{
				currentState_ = FSMState::Idle;
				if (this->idleWandering)
				{
					setWorldState(Action::NEED_WANDERING, true);
					setGoalState(Action::NEED_WANDERING, false);
				}
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
			if (currentActions.empty())
			{
				currentState_ = FSMState::Idle;
				break;
			}

			auto action = currentActions.back();
			auto transform = gameObject->GetTransform();
			auto current = gameObject->GetTransform()->GetWorldPosition();
			auto target = action->target.get()->GetWorldPosition();

			float dt = getDelta(target);

			auto distanceTo = (target - current).Length();
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
				transform->AdjustPosition(gameObject->GetTransform()->GetForwardVector() * movementSpeed);
			}

			break;
		}
	case FSMState::PerformAction:
		{
			if (currentActions.empty())
			{
				currentState_ = FSMState::Idle;
				break;
			}
			auto action = currentActions.back();
			auto current = gameObject->GetTransform()->GetWorldPosition();
			auto target = action->target.get()->GetWorldPosition();
			action->prePerform(gameObject);

			if (action->IsRequiresInRange() && (target - current).Length() < action->range || !action->
				IsRequiresInRange())
			{
				bool result = action->perform(worldState);
				if (result)
				{
					action->postPerform(gameObject);
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

			break;
		}
	}
}


float AIComponent::getDelta(Vector3 target)
{
	if (dumpTarget != target) dt = 0;
	dt += rotationSpeed;
	if (dt > 1) dt = 0;
	dumpTarget = target;
	return dt;
}

GameObject* AIComponent::getPlayer()
{
	GameObject* player = nullptr;
	for (auto it = otherObjects->begin(); it != otherObjects->end(); ++it)
	{
		auto yay = it;
		if (it->get()->GetName() == "MainCamera")
		{
			player = it->get();
			break;
		}
	}
	return player;
}
