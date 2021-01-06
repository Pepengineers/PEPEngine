#include "AMaterial.h"
#include "Material.h"


void PEPEngine::Common::AMaterial::Serialize(json& j)
{
	SerializeIDAndType(j);

	const auto  materialFileName = Asset::GetFilePath(*this, DEFAULT_EXTENSION);

	json materialJson;

	if(material == nullptr){
		material = std::make_shared<Material>();
	}

	material->Serialize(materialJson);

	Asset::WriteToFile(materialFileName, materialJson);


}

void PEPEngine::Common::AMaterial::Deserialize(json& j)
{
	DeserializeIDAndType(j);

	const auto materialFileName = Asset::GetFilePath(*this, DEFAULT_EXTENSION);
	json materialJson;
	Asset::ReadFromFile(materialFileName, materialJson);

	material = std::make_shared<Material>();
	material->Deserialize(materialJson);
}

