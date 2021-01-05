#include "Texture.h"
#include "AssetDatabase.h"

namespace PEPEngine::Common
{
	void Texture::Serialize(std::filesystem::path pathToFile)
	{
		json j;

		SerializeIDAndType(j);
		WriteToFile(pathToFile, j);
	}

	
	
	void Texture::Deserialize(std::filesystem::path pathToFile)
	{
		json j;
		ReadFromFile(pathToFile, j);
		DeserializeIDAndType(j);

		const auto file = FindNativeFile(pathToFile);		

		if (file.has_filename())
		{
			//texture = AssetDatabase::LoadTextureFromFile(file);
		}
		else
		{
			AssetDatabase::RemoveAsset(this);
		}
	}
}