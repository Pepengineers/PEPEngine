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
			std::vector<std::shared_ptr<PEPEngine::Common::GameObject>>* otherObjects;

			FSMState currentState_;
			AIType currentAIType;
			bool idleWandering = true;

			void Serialize(json& j) override;;

			void Deserialize(json& j) override;;

		public:

			SERIALIZE_FROM_JSON(AIComponent)
			void Initialize();
			AIComponent();
			void SetActionList();
			void SetStartWorldState();
			void setWorldState(int, bool);
			void setGoalState(int, bool);
			float getDistanceToPlayer();
			void addGlobalState(std::vector<std::shared_ptr<PEPEngine::Common::GameObject>>* objects);
			PEPEngine::Common::GameObject* getPlayer();
			void Update() override;;
			void preUpdate();

		private:
			float dt;
			float rotationSpeed;
			float movementSpeed;
			Vector3 dumpTarget;
			float getDelta(Vector3);
		};
