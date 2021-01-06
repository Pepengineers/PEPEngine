#include "Level.h"
#include "AssetDatabase.h"
#include "Scene.h"

PEPEngine::Common::Level::Level(): Asset(AssetType::Level)
{
}

std::shared_ptr<PEPEngine::Common::Scene> PEPEngine::Common::Level::GetScene() const
{
	if (scene)
		return scene;

	return nullptr;
}


void PEPEngine::Common::Level::Serialize(json& j)
{

	SerializeIDAndType(j);

	const auto  sceneFileName = Asset::GetFilePath(*this, DEFAULT_EXTENSION);
	
	json sceneJson;

	if(scene == nullptr)
	{
		scene = std::make_shared<Scene>();
	}
	
	scene->Serialize(sceneJson);

	Asset::WriteToFile(sceneFileName, sceneJson);
}

void PEPEngine::Common::Level::Deserialize(json& j)
{
	DeserializeIDAndType(j);
	
	const auto  sceneFileName = Asset::GetFilePath(*this, DEFAULT_EXTENSION);
	json sceneJson;
	Asset::ReadFromFile(sceneFileName, sceneJson);

	scene.reset();
	scene = std::make_shared<Scene>();
	scene->Deserialize(sceneJson);
}
