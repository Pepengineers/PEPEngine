#pragma once
#include "Asset.h"
#include "GTexture.h"

namespace PEPEngine::Common
{
	class ATexture : public Asset
	{
		std::shared_ptr<Graphics::GTexture> texture;

	public:

		ATexture() : Asset(AssetType::Type::Image)
		{
		}	

		void Serialize(json& j) override;

		void Deserialize(json& pathToFile) override;
	public:
		inline std::shared_ptr<Graphics::GTexture> GetGTexture() const{
			return texture;
		}
	private:
		inline static std::shared_ptr<ATexture> defaultAlbedo;
		inline static std::shared_ptr<ATexture> defaultNormal;
		friend class AssetDatabase;
	};


}
