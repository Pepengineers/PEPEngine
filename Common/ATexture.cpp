#include "ATexture.h"
#include "AssetDatabase.h"

namespace PEPEngine::Common
{
	void ATexture::Serialize(json& j)
	{
		SerializeIDAndType(j);
	}


	void ATexture::Deserialize(json& j)
	{
		DeserializeIDAndType(j);

		const auto file = FindNativeFile(pathToFile);

		if (file.has_filename())
		{
			texture = AssetDatabase::LoadTextureFromFile(file);

			texture->SetName(GetName());
		}
		else
		{
			AssetDatabase::RemoveAsset(this);
		}
	}

	std::shared_ptr<ATexture> ATexture::GetDefaultAlbedo()
	{
		return defaultAlbedo;
	}

	std::shared_ptr<ATexture> ATexture::GetDefaultNormal()
	{
		return defaultNormal;
	}
}
