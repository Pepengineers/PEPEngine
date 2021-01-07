#include "AMaterial.h"
#include "Material.h"


PEPEngine::Common::AMaterial::AMaterial() noexcept: Asset(AssetType::Material)
{
}

PEPEngine::Common::AMaterial::AMaterial(const uint64_t id, std::shared_ptr<Material> material): Asset(AssetType::Material),
                                                                                                material(material)
{
	ID = id;
}

void PEPEngine::Common::AMaterial::Serialize(json& j)
{
	SerializeIDAndType(j);

	const auto  materialFileName = FindNativeFile(pathToFile);

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

	const auto materialFileName = FindNativeFile(pathToFile);
	json materialJson;
	Asset::ReadFromFile(materialFileName, materialJson);

	material = std::make_shared<Material>();
	material->Deserialize(materialJson);
}

std::shared_ptr<PEPEngine::Common::Material> PEPEngine::Common::AMaterial::GetMaterial() const
{
	return material;
}

std::shared_ptr<PEPEngine::Common::AMaterial> PEPEngine::Common::AMaterial::GetDefaultMaterial()
{
	return defaultMaterial;
}

