#pragma once
#include "Asset.h"

namespace PEPEngine::Common
{
	class Scene;
	class AScene : public Asset
	{
	private:
		std::shared_ptr<Scene> scene;
		
	public:
		static inline const std::wstring DEFAULT_EXTENSION = L".scene";
		
		AScene();

		std::shared_ptr<Scene> GetScene() const;

		void Serialize(json& j) override;
		void Deserialize(json& j) override;
	};
}



