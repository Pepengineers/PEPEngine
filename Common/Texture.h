#pragma once
#include "Asset.h"
#include "GTexture.h"

namespace PEPEngine::Common
{
	class Texture : public Asset
	{
		std::shared_ptr<Graphics::GTexture> texture;

	protected:

		Texture(): Asset()
		{
		}

	public:

		Texture(unsigned long long id, const std::filesystem::path pathToFile): Asset(id, pathToFile, AssetType::Image)
		{
		}

		void Serialize(json& j) override;

		void Deserialize(json& pathToFile) override;
	};
}
