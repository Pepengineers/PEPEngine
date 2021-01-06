#pragma once
#include "Asset.h"

namespace PEPEngine::Common
{
	class Scene;
	class Level : public Asset
	{
	private:
		std::shared_ptr<Scene> scene;
		static inline const std::wstring DEFAULT_EXTENSION = L".scene";
	public:
		Level();

		std::shared_ptr<Scene> GetScene() const;

		void Serialize(json& j) override;
		void Deserialize(json& j) override;
	};
}



