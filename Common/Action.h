#pragma once
#include "Transform.h"
#include "WorldState.h"
#include "GameObject.h"
#include <memory>
#include <string>
#include <unordered_map>


		class Action
		{


		protected:
			std::string name_; // The human-readable action name
			int cost_; // The numeric cost of this action

			// Preconditions are things that must be satisfied before this
			// action can be taken. Only preconditions that "matter" are here.
			std::unordered_map<int, bool> preconditions_;

			// Effects are things that happen when this action takes place.
			std::unordered_map<int, bool> effects_;
			bool requiresInRange = false;
			bool inRange = false;
			bool isDone = false;


		public:
			Action();
			Action(std::string name, int cost);

			enum ActionList
			{
				POKE_A,
				POKE_B,
				POKE_C,
				WANDERING,
				PICK_UP,
				ATTACK,
			};
			enum Conditions
			{
				HAS_WEAPON,
				NEED_ATTACK,
				NEED_FLEE,
				NEED_WANDERING,
				TARGET_ALIVE,
			};


			bool perform(WorldState& ws);

			[[nodiscard]] bool IsRequiresInRange() const;
			[[nodiscard]] bool getInRange() const;
			[[nodiscard]] bool getIsDone() const;
			void setInRange(bool);
			void setRequiresInRange(bool);
			void setCost(int);
			


			virtual void prePerform(PEPEngine::Common::GameObject*) =0;
			virtual void postPerform(PEPEngine::Common::GameObject*) =0;
			Vector3 getSteering();
			std::shared_ptr<PEPEngine::Common::Transform> target;
			float range = 2;

			/**
			 Is this action eligible to operate on the given worldstate?
			 @param ws the worldstate in question
			 @return true if this worldstate meets the preconditions
			 */
			bool operableOn(const WorldState& ws) const;

			/**
			 Act on the given worldstate. Will not check for "eligiblity" and will happily
			 act on whatever worldstate you provide it.
			 @param the worldstate to act on
			 @return a copy worldstate, with effects applied
			 */
			WorldState actOn(const WorldState& ws) const;

			/**
			 Set the given precondition variable and value.
			 @param key the name of the precondition
			 @param value the value the precondition must hold
			 */
			void setPrecondition(const int key, const bool value)
			{
				preconditions_[key] = value;
			}

			/**
			 Set the given effect of this action, in terms of variable and new value.
			 @param key the name of the effect
			 @param value the value that will result
			 */
			void setEffect(const int key, const bool value)
			{
				effects_[key] = value;
			}

			int cost() const { return cost_; }

			std::string name() const { return name_; }


		};
