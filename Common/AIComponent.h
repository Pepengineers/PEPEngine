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

	std::vector<PEPEngine::goap::Action> currentActions;
	std::vector<PEPEngine::goap::Action> availableActions;

	FSMState currentState_;

public:

	AIComponent();
	void SetActionList();
	void SetWorldState();
	void Update() override;
	void PopulateDrawCommand(std::shared_ptr<PEPEngine::Graphics::GCommandList> cmdList) override;
};
