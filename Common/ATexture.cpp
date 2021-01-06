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
		}
		else
		{
			AssetDatabase::RemoveAsset(this);
		}
	}
}
