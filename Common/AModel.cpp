#include "AModel.h"
#include "GModel.h"
#include "AssetDatabase.h"

std::shared_ptr<PEPEngine::Common::GModel> PEPEngine::Common::AModel::GetGModel()
{
	return gModel;
}

void PEPEngine::Common::AModel::Serialize(json& j)
{
	SerializeIDAndType(j);

	assert(gModel != nullptr);

	gModel->Serialize(j);

}

void PEPEngine::Common::AModel::Deserialize(json& j)
{
	DeserializeIDAndType(j);

	const auto file = FindNativeFile(pathToFile);

	if (file.has_filename())
	{
		gModel = AssetDatabase::LoadModelFromFile(file);
		gModel->Deserialize(j);
	}
	else
	{
		AssetDatabase::RemoveAsset(this);
	}
}
