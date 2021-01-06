#ifndef A_MATERIAL_H
#define A_MATERIAL_H

#include "Asset.h"

namespace PEPEngine::Common {
	class Material;
	class AMaterial : public Asset {
	public:
		inline AMaterial() noexcept : Asset(AssetType::Material) {}
	public:
		void Serialize(json& j) override;
		void Deserialize(json& j) override;
	private:
		std::shared_ptr<Material> material;
		static const inline std::wstring DEFAULT_EXTENSION = L".mat";
		inline static std::shared_ptr<AMaterial> defaultMaterial;
		friend class AssetDatabase;
	};

}

#endif
