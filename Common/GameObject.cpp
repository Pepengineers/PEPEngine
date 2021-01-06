#include "pch.h"
#include "GameObject.h"
#include <utility>

#include "AssetDatabase.h"
#include "Component.h"
#include "ModelRenderer.h"
#include "Transform.h"

namespace PEPEngine::Common
{
	using namespace Allocator;
	using namespace Graphics;
	using namespace Utils;

	GameObject::GameObject() : GameObject("Game Object")
	{
	};

	GameObject::GameObject(std::string name)
		: GameObject(std::move(name), Vector3::Zero,
		             Vector3::One, Quaternion::Identity)
	{
	}

	GameObject::
	GameObject(std::string name, Vector3 position, Vector3 scale, Quaternion rotate) :  name(
		std::move(name))
	{
		
		
		transform = std::make_shared<Transform>(position, rotate, scale);

		AddComponent(transform);
	}

	void GameObject::Update()
	{
		for (auto& component : components)
		{
			component->Update();
		}
	}


	std::shared_ptr<Transform> GameObject::GetTransform() const
	{
		return transform;
	}

	std::shared_ptr<ModelRenderer> GameObject::GetRenderer()
	{
		if (renderer == nullptr)
		{
			for (auto&& component : components)
			{
				const auto comp = dynamic_cast<ModelRenderer*>(component.get());
				if (comp)
				{
					renderer = std::static_pointer_cast<ModelRenderer>(component);
					break;
				}
			}
		}

		return renderer;
	}

	void GameObject::SetScale(float scale) const
	{
		transform->SetScale(Vector3(scale, scale, scale));
	}

	void GameObject::SetScale(Vector3& scale) const
	{
		transform->SetScale(scale);
	}

	std::string& GameObject::GetName()
	{
		return name;
	}

	void GameObject::Serialize(json& j)
	{
		type = AssetType::None;
		ID = AssetDatabase::GenerateID();
		
		SerializeIDAndType(j);
		
		j["ComponentCount"] = components.size();

		auto componentsArray = json::array();

		for (int i = 0; i < components.size(); ++i)
		{
			json componentData;
			components[i]->Serialize(componentData);
			componentsArray.push_back(componentData);
		}

		j["Components"] = componentsArray;
		
	};

	void GameObject::Deserialize(json& j)
	{
		DeserializeIDAndType(j);
		
		UINT size;
		assert(TryReadVariable<UINT>(j, "ComponentCount", &size));
		
		std::vector<UINT> componentID;

		json array = j["Components"];
		
		for (int i = 0; i < size; ++i)
		{
			std::wstring id;
			assert(TryReadVariable<std::wstring>(array[i], "Type", &id));

			if(id == Transform::ComponentID)
			{
				transform->Deserialize(array[i]);
			}
			else
			{
				auto component = Component::CreateFromFile(array[i], id);
				AddComponent(std::move(component));
			}
		}		
	};
}
