#pragma once
#include "Asset.h"
#include "GTexture.h"

namespace PEPEngine::Common
{
	class Texture : public Asset
	{
		std::shared_ptr<Graphics::GTexture> texture;

	public:

		Texture() : Asset(AssetType::Type::Image)
		{
		}	

		void Serialize(json& j) override;

		void Deserialize(json& pathToFile) override;
	};
}
