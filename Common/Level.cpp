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

bool replace(std::wstring& str, const std::wstring& from, const std::wstring& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::wstring::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

std::filesystem::path PEPEngine::Common::Level::GetSceneFilePath() const
{
	auto file = pathToFile.filename().wstring();

	replace(file, pathToFile.extension().wstring(), L".scene");


	
	return std::filesystem::path( pathToFile.parent_path().concat("\\").concat(file));
}


void PEPEngine::Common::Level::Serialize(json& j)
{
	if (type == AssetType::None)
		type = AssetType::Level;

	if (ID == UINT64_MAX)
		ID = AssetDatabase::GenerateID();

	SerializeIDAndType(j);


	const auto  sceneFileName = GetSceneFilePath();
	
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
	
	const auto  sceneFileName = GetSceneFilePath();
	json sceneJson;
	Asset::ReadFromFile(sceneFileName, sceneJson);

	scene.reset();
	scene = std::make_shared<Scene>();
	scene->Deserialize(sceneJson);
}
