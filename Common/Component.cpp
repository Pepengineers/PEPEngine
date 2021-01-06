#include "pch.h"
#include "Component.h"

namespace PEPEngine::Common
{
	Component::Component(json& json)
	{
	}

	std::shared_ptr<Component> Component::CreateFromFile(json& json, const std::wstring& componentGUID)
	{
		return (componentsFactory[componentGUID](json));
	}

	Component::~Component() = default;

	Component::Component() : Asset()
	{
	}

}
