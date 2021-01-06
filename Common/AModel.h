#pragma once

#include "Asset.h"

namespace PEPEngine::Common {

	class GModel;

	class AModel : public Asset
	{
	public:
		inline AModel() noexcept : Asset(AssetType::Model) {}

	public:
		void Serialize(json& j) override;
		void Deserialize(json& j) override;
		std::shared_ptr<GModel> GetGModel();
	private:
		std::shared_ptr<GModel> gModel;
	};

}

