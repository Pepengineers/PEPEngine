#pragma once
#include "AModel.h"
#include "ATexture.h"
#include "GModel.h"
#include "GTexture.h"
#include "NativeModel.h"
#include "ShaderBuffersData.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"

namespace PEPEngine::Common
{
	class AssimpModelLoader
	{
		static std::vector<Vertex> GetVertices(aiMesh* mesh);

		static std::vector<unsigned long> GetIndices(aiMesh* mesh);

		static std::shared_ptr<NativeMesh> CreateSubMesh(aiMesh* mesh, const std::string& modelName);

		static void RecursivlyLoadMeshes(std::shared_ptr<NativeModel> model, aiNode* node, const aiScene* scene);

	public:
		static std::shared_ptr<GModel> CreateModelFromFile(const std::shared_ptr<GCommandList>& cmdList,
		                                                   const std::filesystem::path& filePath);

		static std::shared_ptr<ATexture> LoadTextureFromAiMaterial(const aiMaterial* material, const aiTextureType type,
		                                                           const std::filesystem::path& directory, std::shared_ptr<AModel> model);
		static std::vector<std::shared_ptr<AMaterial>> FindAndCreateMaterialFromModelFile(const std::shared_ptr<GCommandList>& cmdList,
		                                                                                  const std::string& filePath, std::shared_ptr<AModel> model);
	};
}
