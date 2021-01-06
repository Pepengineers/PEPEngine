#pragma once
#include "Asset.h"

namespace PEPEngine::Common
{
	class Scene;
	class Level :		public Asset
	{
	private:
		std::shared_ptr<Scene> scene;


		std::filesystem::path GetSceneFilePath() const;
	public:
		Level();

		std::shared_ptr<Scene> GetScene() const;

		void Serialize(json& j) override;
		void Deserialize(json& j) override;
	};
}



