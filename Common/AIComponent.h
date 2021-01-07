#pragma once
#include "WorldState.h"
#include "GameObject.h"
#include "Planner.h"


class AIComponent : public PEPEngine::Common::Component
		{
			enum class FSMState
			{
				Idle,
				MoveTo,
				PerformAction,
			};
			enum class AIType
			{
				Aggressive,
				Passive,
				Frightened

			};

			WorldState worldState;
			WorldState goal;
			Planner planner;

			std::vector<Action*> currentActions;
			std::vector<Action*> availableActions;
			PEPEngine::Allocator::custom_vector<std::shared_ptr<PEPEngine::Common::GameObject>>* otherObjects;

			FSMState currentState_;
			AIType currentAIType;
			bool idleWandering = true;

		public:
			AIComponent();
			void SetActionList();
			void SetStartWorldState();
			void setWorldState(int, bool);
			void setGoalState(int, bool);
			float getDistanceToPlayer();
			void addGlobalState(PEPEngine::Allocator::custom_vector<std::shared_ptr<PEPEngine::Common::GameObject>>& objects);
			PEPEngine::Common::GameObject* getPlayer();
			void Update() override;;
			void preUpdate();
			void PopulateDrawCommand(std::shared_ptr<PEPEngine::Graphics::GCommandList> cmdList) override;;
	void Serialize(json& j) override
	{
		j["Type"] = ComponentID;
	};
void Deserialize(json& j) override
	{		
		Initialize();
	};



		private:
			float dt;
			float rotationSpeed;
			float movementSpeed;
			Vector3 dumpTarget;
			float getDelta(Vector3);
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
