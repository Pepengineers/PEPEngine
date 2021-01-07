#ifndef A_MATERIAL_H
#define A_MATERIAL_H

#include "Asset.h"

namespace PEPEngine::Common
{
	class Material;

	class AMaterial : public Asset
	{
	public:
		AMaterial() noexcept;

		AMaterial(uint64_t id, std::shared_ptr<Material> material);

	public:

		static const inline std::wstring DEFAULT_EXTENSION = L".mat";
		void Serialize(json& j) override;
		void Deserialize(json& j) override;

		std::shared_ptr<Material> GetMaterial() const;;

		static std::shared_ptr<AMaterial> GetDefaultMaterial();

	private:
		std::shared_ptr<Material> material;
		inline static std::shared_ptr<AMaterial> defaultMaterial;
		friend class AssetDatabase;
	};
}

#endif
