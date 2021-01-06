#pragma once
#include "Component.h"
#include "goap/Planner.h"
#include "goap/WorldState.h"


class AIComponent : public PEPEngine::Common::Component
{
	enum class FSMState
	{
		Idle,
		MoveTo,
		PerformAction,
	};

	PEPEngine::goap::WorldState worldState;
	PEPEngine::goap::WorldState goal;
	PEPEngine::goap::Planner planner;

	std::vector<PEPEngine::goap::Action*> currentActions;
	std::vector<PEPEngine::goap::Action*> availableActions;

	FSMState currentState_;

	

	void Serialize(json& j) override
	{
		j["Type"] = ComponentID;
	};

	void Deserialize(json& j) override
	{		
		Initialize();
	};

public:

	SERIALIZE_FROM_JSON(AIComponent)
	
	void Initialize();
	AIComponent();
	void SetActionList();
	void SetWorldState();
	void Update() override;

private:
	float dt;
	Vector3 dumpTarget;
};
