#pragma once
#include "Renderer.h"
#include "GModel.h"

class Transform;
class GCommandList;

class ModelRenderer : public Renderer
{	

	ObjectConstants constantData{};
	
	std::unique_ptr<ConstantBuffer<ObjectConstants>> modelDataBuffer = nullptr;
	std::unique_ptr<ConstantBuffer<MaterialConstants>> materialsDataBuffer = nullptr;
	
	custom_vector<std::shared_ptr<Material>> meshesMaterials = MemoryAllocator::CreateVector< std::shared_ptr<Material>>();
	
	
protected:
		
	void Draw(std::shared_ptr<GCommandList> cmdList) override;

	void Update() override;

public:

	std::shared_ptr<GModel> model = nullptr;

	void SetModel(std::shared_ptr<GModel> asset);

	UINT GetMeshesCount() const;

	void SetMeshMaterial(UINT index, const std::shared_ptr<Material> material);
};
