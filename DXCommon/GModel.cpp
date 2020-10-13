#include "pch.h"
#include "GModel.h"
#include "GMesh.h"
#include "NativeModel.h"

namespace DX
{
	namespace Common
	{
		UINT GModel::GetMeshesCount() const
		{
			return model->GetMeshesCount();
		}

		std::shared_ptr<Material> GModel::GetMeshMaterial(UINT index)
		{
			return meshesMaterials[index];
		}

		std::shared_ptr<GMesh> GModel::GetMesh(UINT index)
		{
			return gmeshes[index];
		}

		std::wstring GModel::GetName() const
		{
			return model->GetName();
		}

		GModel::GModel(std::shared_ptr<NativeModel> model, std::shared_ptr<GCommandList> uploadCmdList): model(model)
		{
			if (meshesMaterials.size() < model->GetMeshesCount())
			{
				meshesMaterials.resize(model->GetMeshesCount());
			}

			for (int i = 0; i < model->GetMeshesCount(); ++i)
			{
				auto nativeMesh = model->GetMesh(i);
				gmeshes.push_back(std::make_shared<GMesh>(nativeMesh, uploadCmdList));
			}
		}

		void GModel::SetMeshMaterial(UINT index, const std::shared_ptr<Material> material)
		{
			meshesMaterials[index] = material;
		}

		GModel::GModel(const GModel& copy) : model(copy.model)
		{
			gmeshes.resize(copy.gmeshes.size());

			for (int i = 0; i < gmeshes.size(); ++i)
			{
				gmeshes[i] = std::move(std::make_shared<GMesh>(*copy.gmeshes[i]));
			}
		}

		GModel::~GModel() = default;

		void GModel::Draw(std::shared_ptr<GCommandList> cmdList)
		{
			for (auto&& mesh : gmeshes)
			{
				mesh->Draw(cmdList);
			}
		}

		std::shared_ptr<GModel> GModel::Dublicate(std::shared_ptr<GCommandList> otherDeviceCmdList) const
		{
			auto dublciate = std::make_shared<GModel>(model, otherDeviceCmdList);
			dublciate->scaleMatrix = scaleMatrix;
			return dublciate;
		}
	}
}
