#pragma once


namespace PEPEngine
{
	namespace Common
	{
		class GameObject;

		class Component
		{
		public:
			virtual ~Component() = default;

			GameObject* gameObject = nullptr;

			Component();

			virtual void Update() = 0;
		};
	}
}
