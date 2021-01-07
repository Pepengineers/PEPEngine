#pragma once

#include "Asset.h"

namespace PEPEngine::Common {

	class GModel;

	class AModel : public Asset
	{
	public:
		AModel() noexcept;

		AModel(uint64_t id, std::shared_ptr<GModel> model);

	public:
		void Serialize(json& j) override;
		void Deserialize(json& j) override;
		std::shared_ptr<GModel> GetGModel() const;
	private:
		std::shared_ptr<GModel> gModel;

		friend class AssetDatabase;
	};

}

