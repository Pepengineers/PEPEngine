#include "AModel.h"
#include "GModel.h"
#include "AssetDatabase.h"

std::shared_ptr<PEPEngine::Common::GModel> PEPEngine::Common::AModel::GetGModel() const
{
	return gModel;
}

PEPEngine::Common::AModel::AModel() noexcept: Asset(AssetType::Model)
{
}

PEPEngine::Common::AModel::AModel(uint64_t id, std::shared_ptr<GModel> model): Asset(AssetType::Model), gModel(model)
{
	ID = id;
}

void PEPEngine::Common::AModel::Serialize(json& j)
{
	SerializeIDAndType(j);

	if(gModel != nullptr)gModel->Serialize(j);

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
