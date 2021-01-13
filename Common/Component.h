#pragma once
#include <functional>

#include "unordered_map"
#include "Asset.h"

#define SERIALIZE_FROM_JSON_(type, parent, ...) \
	static std::shared_ptr<type> CreateFromJSON(json& json)\
	{\
	return std::make_shared<type>(json);\
	}\
	static std::string GetComponentID()\
	{\
	auto id = #type;\
	componentsFactory[id] = CreateFromJSON;\
	return id;\
	}\
	inline static std::string ComponentID = GetComponentID();\
	void InitializeFromJson(json& json)\
	{\
	Deserialize(json);\
	}\
	type(json& json): parent(json) { InitializeFromJson(json); }


#define INVOKE_SERIALIZE_(ARGS) SERIALIZE_FROM_JSON_ ARGS

#define SERIALIZE_FROM_JSON(...) INVOKE_SERIALIZE_((__VA_ARGS__, Component))

namespace PEPEngine
{
	namespace Common
	{
		class GameObject;

		class Component : public Asset
		{
		protected:

			inline static std::unordered_map<std::string, std::function<std::shared_ptr<Component>(json&)>>
			componentsFactory{};


			Component(json& json);

		public:

			static std::shared_ptr<Component> CreateFromFile(json& json, const std::string& componentGUID);

			virtual ~Component();

			GameObject* gameObject = nullptr;

			virtual void OnGUI()
			{
			};

			Component();

			virtual void Update() = 0;
		public:
		};
	}
}
