#include "pch.h"
#include "GModel.h"
#include "GMesh.h"
#include "NativeModel.h"

namespace PEPEngine::Common
{
	std::shared_ptr<GDevice> GModel::GetDevice() const
	{
		return device;
	}

	UINT GModel::GetMeshesCount() const
	{
		return model->GetMeshesCount();
	}

	std::shared_ptr<GMesh> GModel::GetMesh(UINT index)
	{
		return gmeshes[index];
	}

	std::wstring GModel::GetName() const
	{
		return model->GetName();
	}

	GModel:: GModel(std::shared_ptr<NativeModel> model, std::shared_ptr<GCommandList> uploadCmdList): model(model)
	{		
		for (int i = 0; i < model->GetMeshesCount(); ++i)
		{
			auto nativeMesh = model->GetMesh(i);
			gmeshes.push_back(std::make_shared<GMesh>(nativeMesh, uploadCmdList));
		}

		device = uploadCmdList->GetDevice();
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

	void GModel::Render(std::shared_ptr<GCommandList> cmdList)
	{
		for (auto&& mesh : gmeshes)
		{
			mesh->Render(cmdList);
		}
	}

	std::shared_ptr<GModel> GModel::Dublicate(std::shared_ptr<GCommandList> otherDeviceCmdList) const
	{
		auto dublciate = std::make_shared<GModel>(model, otherDeviceCmdList);
		dublciate->scaleMatrix = scaleMatrix;
		return dublciate;
	}
}
