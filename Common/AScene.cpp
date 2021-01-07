#include "AScene.h"
#include "AssetDatabase.h"
#include "Scene.h"

PEPEngine::Common::AScene::AScene(): Asset(AssetType::Level)
{
}

std::shared_ptr<PEPEngine::Common::Scene> PEPEngine::Common::AScene::GetScene() const
{
	if (scene)
		return scene;

	return nullptr;
}


void PEPEngine::Common::AScene::Serialize(json& j)
{

	SerializeIDAndType(j);

	const auto  sceneFileName = std::filesystem::path(pathToFile.parent_path().wstring() + L"\\" + GetName() + DEFAULT_EXTENSION );
	
	json sceneJson;

	if(scene == nullptr)
	{
		scene = std::make_shared<Scene>();
	}
	
	scene->Serialize(sceneJson);

	Asset::WriteToFile(sceneFileName, sceneJson);
}

void PEPEngine::Common::AScene::Deserialize(json& j)
{
	DeserializeIDAndType(j);
	
	const auto  sceneFileName = std::filesystem::path(pathToFile.parent_path().wstring() + L"\\" + GetName() + DEFAULT_EXTENSION);
	json sceneJson;
	Asset::ReadFromFile(sceneFileName, sceneJson);

	scene.reset();
	scene = std::make_shared<Scene>();
	scene->Deserialize(sceneJson);
}
