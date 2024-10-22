#pragma once

#include "Asset.h"
#include "SimpleMath.h"
#include "MemoryAllocator.h"

using namespace DirectX::SimpleMath;

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;

		class ModelRenderer;
		class Component;
		class Transform;
		class Renderer;

		class GameObject : public Asset
		{
		public:

			GameObject();

			GameObject(std::string name);

			GameObject(std::string name, Vector3 position, Vector3 scale, Quaternion rotate);

			void virtual Update();

			std::shared_ptr<Transform> GetTransform() const;

			std::shared_ptr<ModelRenderer> GetRenderer();

			template <class T = Component>
			void AddComponent(std::shared_ptr<T> component);

			template <class T = Component>
			std::shared_ptr<T> GetComponent();

			void SetScale(float scale) const;

			void SetScale(Vector3& scale) const;

			std::string& GetName();

			void Serialize(json& j) override;

			void Deserialize(json& j) override;

		protected:

			std::vector<std::shared_ptr<Component>> components;
			std::shared_ptr<Transform> transform = nullptr;
			std::shared_ptr<ModelRenderer> renderer = nullptr;
			std::string name;
		};

		template <class T = Component>
		void GameObject::AddComponent(std::shared_ptr<T> component)
		{
			component->gameObject = this;
			components.push_back(component);
		}

		template <class T = Component>
		std::shared_ptr<T> GameObject::GetComponent()
		{
			for (auto&& component : components)
			{
				auto ptr = component.get();
				if (dynamic_cast<T*>(ptr) != nullptr)
				{
					return std::static_pointer_cast<T>(component);
				}
			}
			return nullptr;
		}
	}
}
