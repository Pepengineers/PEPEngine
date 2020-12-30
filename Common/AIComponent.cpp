#include "AIComponent.h"

#include "DoActionA.h"
#include "DoActionB.h"
#include "GameObject.h"
#include "Transform.h"

enum test { TARGET_ACQUIRED, TARGET_DEAD,IN_LOCATION };
AIComponent::AIComponent()
{
	this->SetWorldState();
	this->SetActionList();
	this->currentState_ = FSMState::Idle;
	
}

void AIComponent::SetActionList()
{
	PEPEngine::goap::Action a("move_a", 5);
	a.setEffect(PEPEngine::goap::Action::POKE_A, true);
	a.setRequiresInRange(true);
	a.target->SetPosition(Vector3(0, 0, 0));
	availableActions.push_back(a);

	PEPEngine::goap::Action b("move_b", 5);
	b.setEffect(PEPEngine::goap::Action::POKE_B, true);
	b.setRequiresInRange(true);
	b.target->SetPosition(Vector3(0, 100, 0));
	availableActions.push_back(b);

	PEPEngine::goap::Action c("move_c", 5);
	c.setEffect(PEPEngine::goap::Action::POKE_C, true);
	c.setRequiresInRange(true);
	c.target->SetPosition(Vector3(100, 0, 0));
	availableActions.push_back(c);

}


void AIComponent::SetWorldState()
{
	worldState.setVariable(PEPEngine::goap::Action::POKE_A, false);
	worldState.setVariable(PEPEngine::goap::Action::POKE_B, false);
	worldState.setVariable(PEPEngine::goap::Action::POKE_C, false);
	goal.setVariable(PEPEngine::goap::Action::POKE_A, true);
	goal.setVariable(PEPEngine::goap::Action::POKE_B, true);
	goal.setVariable(PEPEngine::goap::Action::POKE_C, true);
}

void AIComponent::Update()
{
	if(worldState.getVariable(PEPEngine::goap::Action::POKE_A) && worldState.getVariable(PEPEngine::goap::Action::POKE_B) &&worldState.getVariable(PEPEngine::goap::Action::POKE_C))
	{
		worldState.setVariable(PEPEngine::goap::Action::POKE_A, false);
		worldState.setVariable(PEPEngine::goap::Action::POKE_B, false);
		worldState.setVariable(PEPEngine::goap::Action::POKE_C, false);
	}
	switch (currentState_)
	{
	case FSMState::Idle:
		{
		auto plan = planner.plan(worldState, goal, availableActions);

			if(plan.empty())
			{
				currentState_ = FSMState::Idle;
				
			} else
			{
				currentActions = plan;
				currentState_ = FSMState::PerformAction;
			}
			break;
		}
	case FSMState::MoveTo:
		{
		//TODO: Переписать под нормальное передвижение
			
		auto action = &currentActions.back();
		auto transform = gameObject->GetTransform();
		auto current = gameObject->GetTransform()->GetWorldPosition();
		auto target = action->target.get()->GetWorldPosition();
		auto len = (target - current);
		auto lelen = len.Length();

			if(lelen > 0 && lelen <2)
			{
				action->setInRange(true);
				currentState_ = FSMState::PerformAction;
				
			}
			else
			{
				auto dir = target - current;
				dir.Normalize();
				transform->AdjustPosition(dir*0.1);
				
			}
			
			break;
			
		}
	case FSMState::PerformAction:
	{
		
			
			if(currentActions.empty())
			{
				currentState_ = FSMState::Idle;
			}
			else
			{
				auto action = &currentActions.back();
				if (action->IsRequiresInRange() && action->getInRange() || !action->IsRequiresInRange())
				{
					bool result = action->perform(worldState);
					if (result)
					{
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
